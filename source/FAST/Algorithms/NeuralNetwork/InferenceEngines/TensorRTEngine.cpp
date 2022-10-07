#include "TensorRTEngine.hpp"
#include "NvInfer.h"
#include "NvUffParser.h"
#include "NvOnnxParser.h"
#include "NvUtils.h"
#include "NvInferRuntimeCommon.h"
#include <cuda_runtime_api.h>
#include <FAST/Utility.hpp>
#include <fstream>
#include <FAST/Config.hpp>

#define CUDA_CHECK(status)                             \
if(status != 0)                             \
{                                         \
    throw Exception("CUDA failure: " + std::string(cudaGetErrorString(cudaGetLastError())));    \
}                                         \


namespace fast {

/**
 * Logger object for TensorRT
 */
class Logger : public nvinfer1::ILogger {
    void log(Severity severity, const char* msg) noexcept override {
        switch(severity) {
            case Severity::kINFO:
                Reporter::info() << "[TensorRT] " << msg << Reporter::end();
                break;
            case Severity::kERROR:
            case Severity::kINTERNAL_ERROR:
                Reporter::error() << "[TensorRT] " << msg << Reporter::end();
                break;
            case Severity::kWARNING:
                Reporter::warning() << "[TensorRT] " << msg << Reporter::end();
                break;
        }
    }
} gLogger;

// Call destroy when an object is deleted
struct Destroy {
    template <typename T>
    void operator()(T* obj) const {
        if (obj)
            obj->destroy();
    }
};

static TensorShape getTensorShape(nvinfer1::Dims dims) {
    TensorShape shape;
    for(int j = 0; j < dims.nbDims; ++j) {
        auto size = dims.d[j];
        shape.addDimension(size);
    }
    return shape;
}

inline unsigned int elementSize(nvinfer1::DataType t) {
    switch (t)
    {
        case nvinfer1::DataType::kINT32:
            // Fallthrough, same as kFLOAT
        case nvinfer1::DataType::kFLOAT: return 4;
        case nvinfer1::DataType::kHALF: return 2;
        case nvinfer1::DataType::kINT8: return 1;
    }
    assert(0);
    return 0;
}

void* safeCudaMalloc(size_t memSize) {
    void* deviceMem;
    CUDA_CHECK(cudaMalloc(&deviceMem, memSize));
    if(deviceMem == nullptr) {
        throw Exception("CUDA: Out of memory");
    }
    return deviceMem;
}

inline nvinfer1::Dims shapeToDims(TensorShape shape) {
    nvinfer1::Dims d;
    d.nbDims = shape.getDimensions();
    for(int i = 0; i < shape.getDimensions(); ++i) {
        d.d[i] = shape[i];
    }
    return d;
}

void TensorRTEngine::run() {
    const int nbBindings = m_engine->getNbBindings();

    // Get batch size
    const int batchSize = mInputNodes.begin()->second.data->getShape()[0];
    if(batchSize > m_maxBatchSize) {
        reportWarning() << "Batch is larger than the max batch size" << reportEnd();
    }

    if(m_cudaBuffers.empty() || m_currentBatchSize != batchSize) {
        m_currentBatchSize = batchSize;
        // Initialize cuda buffers. WARNING: This assumes that shapes (except batch size) do not change during execution
        for(int i = 0; i < nbBindings; ++i) {
            auto name = m_engine->getBindingName(i);
            TensorShape shape;
            if(m_engine->bindingIsInput(i) && mInputNodes.count(name) > 0) {
                m_inputIndexes[name] = i;
                shape = mInputNodes[name].shape;
            } else if(!m_engine->bindingIsInput(i) && mOutputNodes.count(name) > 0) {
                m_outputIndexes[name] = i;
                shape = mOutputNodes[name].shape;
            } else {
                shape = getTensorShape(m_engine->getBindingDimensions(i));
            }
            shape[0] = batchSize;
            // Allocate data
            nvinfer1::DataType dtype = m_engine->getBindingDataType(i);
            m_cudaBuffers.push_back(safeCudaMalloc(shape.getTotalSize() * elementSize(dtype)));
        }
    }

    // Allocate data for each input and copy data to it
    for(const auto& inputNode : mInputNodes) {
        auto tensor = inputNode.second.data;
        auto access = tensor->getAccess(ACCESS_READ);
        float* tensorData = access->getRawData();
        const int index = m_inputIndexes.at(inputNode.first);
        auto dtype = m_engine->getBindingDataType(index);
        auto shape = inputNode.second.shape;
        shape[0] = batchSize;
        m_context->setBindingDimensions(index, shapeToDims(shape));
        CUDA_CHECK(cudaMemcpy(m_cudaBuffers[index], tensorData,
                              shape.getTotalSize() * elementSize(dtype),
                              cudaMemcpyHostToDevice));
        reportInfo() << "Finished copying input data to TensorRT" << reportEnd();
    }

    // Execute network
    bool success = m_context->executeV2(m_cudaBuffers.data()); // IMPORTANT: Does not work with uff parser atm
    //bool success = m_context->execute(batchSize, &m_cudaBuffers[0]);
    if(!success)
        throw Exception("TensorRT execute inference failed!");
    reportInfo() << "Finished execute TensorRT" << reportEnd();

    // Transfer output data back
    for(auto& outputNode : mOutputNodes) {
        const auto name = outputNode.first;
        auto shape = outputNode.second.shape;
        shape[0] = batchSize;
        const int index = m_outputIndexes[name];
        auto dtype = m_engine->getBindingDataType(index);
        reportInfo() << "Processing output node " << name << reportEnd();
        auto outputData = make_uninitialized_unique<float[]>(shape.getTotalSize()); // TODO possible type mismatch issue here..
        CUDA_CHECK(cudaMemcpy(outputData.get(), m_cudaBuffers[index],
                              shape.getTotalSize() * elementSize(dtype),
                              cudaMemcpyDeviceToHost));


        // Get output shape
        nvinfer1::Dims dims = m_engine->getBindingDimensions(index);
        if(shape.getDimensions() == 0)
            throw Exception("Missing shape for output node");

        auto outputTensor = Tensor::create(std::move(outputData), shape);
        outputNode.second.data = outputTensor;
        reportInfo() << "Finished moving data to FAST tensor, TensorRT" << reportEnd();
        reportInfo() << "Finished transfer of output data TensorRT" << reportEnd();
    }
}

void TensorRTEngine::load() {

    const auto filename = getFilename();
    std::size_t hash = std::hash<std::string>{}(filename + std::to_string(m_maxBatchSize)); // Hash the full filename any other parameters
    std::string serializedBinaryFilename = join(Config::getKernelBinaryPath(), getFileName(filename) + "_" + std::to_string(hash) + ".bin");
    std::string serializedCacheFilename = join(Config::getKernelBinaryPath(), getFileName(filename) + "_" + std::to_string(hash) + ".cache");
    bool loadSerializedFile = false;
    if(fileExists(serializedCacheFilename) && fileExists(serializedBinaryFilename)) {
        // Check if date is modified or not
        std::ifstream file(serializedCacheFilename.c_str());
        if(file.fail())
            throw Exception("Failed to read " + serializedCacheFilename);
        std::string line = "";
        std::getline(file, line);
        trim(line);
        std::string modifiedDate = getModifiedDate(filename);
        trim(modifiedDate);
        if(line == modifiedDate) {
            reportInfo() << "[TensorRT] Serialized file " << serializedBinaryFilename << " is up to date." << reportEnd();
            loadSerializedFile = true;
        } else {
            reportInfo() << "[TensorRT] Serialized file " << serializedBinaryFilename << " was not up to date." << reportEnd();
        }

        if(!file.eof()) { // If serialized file has tensorrt version stored in cache
            std::getline(file, line);
            trim(line);
            std::string TensorRTVersion = std::to_string(getInferLibVersion());
            if(line == TensorRTVersion) {
				reportInfo() << "[TensorRT] Serialized file was created with same TensorRT version: " << TensorRTVersion << reportEnd();
            } else {
				loadSerializedFile = false;
				reportWarning() << "[TensorRT] Serialized file was NOT created with same TensorRT version as running. Running version: " << TensorRTVersion << " Created with: " << line << reportEnd();
            }
        }
    }
    if(!loadSerializedFile) {
        reportWarning() << "TensorRT will now perform auto-tuning for your model. This may take a while! But this is only done the first time loading a new model." << reportEnd();
        // File is not serialized: Create it
        if(!fileExists(filename))
            throw FileNotFoundException(filename);
        reportInfo() << "Loading file " << filename << " using TensorRT" << reportEnd();
        std::unique_ptr<nvinfer1::IBuilder, decltype(Destroy())> builder(nvinfer1::createInferBuilder(gLogger),
                                                                         Destroy());
        const auto flags = 1U << static_cast<uint32_t>(nvinfer1::NetworkDefinitionCreationFlag::kEXPLICIT_BATCH);
        std::unique_ptr<nvinfer1::INetworkDefinition, decltype(Destroy())> network;
        std::unique_ptr<nvuffparser::IUffParser, decltype(Destroy())> uffparser(nvuffparser::createUffParser(),
                                                                             Destroy()); // IMPORTANT! UFF parser must be alive even after parse
        network = {builder->createNetworkV2(flags),
                       Destroy()};
        std::unique_ptr<nvonnxparser::IParser, decltype(Destroy())> onnxparser(nvonnxparser::createParser(*network, gLogger),
                                                                           Destroy()); // IMPORTANT! Parser must be alive even after parse

        //builder->setFp16Mode(builder->platformHasFastFp16());

        std::unique_ptr<nvinfer1::IBuilderConfig, decltype(Destroy())> config(builder->createBuilderConfig(), Destroy());
        config->setMaxWorkspaceSize(m_maxWorkspaceSize);
        // Enable FP16 and INT8 with FP32 fallback:
        config->setFlag(nvinfer1::BuilderFlag::kFP16);
        //config->setFlag(nvinfer1::BuilderFlag::kINT8);

        if(filename.substr(filename.size()-4) == ".uff") {
            // Check that input and output nodes are set
            if(mInputNodes.empty() || mOutputNodes.empty())
                throw Exception("Input and output nodes must be defined before loading Uff files using the TensorRT engine");
            reportInfo() << "Assuming file is in UFF format, parsing..." << reportEnd();

            // Setup input nodes
            reportInfo() << "Going through nodes.." << reportEnd();
            for(auto node : mInputNodes) {
                reportInfo() << node.first << reportEnd();
                auto shape = node.second.shape;
                if(shape.getDimensions() == 0)
                    throw Exception("Unknown shape for input node " + node.first +
                                    ". For TensorRT you need to specify the input tensor shape explictly with addInputNode");
                if(shape.getDimensions() == 4)
                    uffparser->registerInput(node.first.c_str(), nvinfer1::Dims3(shape[1], shape[2], shape[3]),
                                          nvuffparser::UffInputOrder::kNCHW);
                if(shape.getDimensions() == 5)
                    throw Exception("More than 4 dimensions input is not supported");
            }
            reportInfo() << "Input nodes finished" << reportEnd();

            // Setup output nodes
            for(auto node : mOutputNodes) {
                uffparser->registerOutput(node.first.c_str());
            }
            reportInfo() << "Output nodes finished" << reportEnd();

            if(!uffparser->parse(filename.c_str(), *network, nvinfer1::DataType::kFLOAT))
                throw Exception("Error parsing UFF file " + filename);

            m_imageOrdering = ImageOrdering::ChannelFirst;
            reportInfo() << "Finished parsing UFF file" << reportEnd();
        } else {
            // Assuming file is ONNX format
            reportInfo() << "Assuming file is in ONNX format, parsing..." << reportEnd();
            bool parsed = onnxparser->parseFromFile(filename.c_str(), 1);
            if(!parsed)
                throw Exception("Unable to parse ONNX file with TensorRT");

            auto profile = builder->createOptimizationProfile();
            // For all inputs, set profiling dimensions
            for(int inputNr = 0; inputNr < network->getNbInputs(); ++inputNr) {
                auto input = network->getInput(inputNr);
                auto dims = input->getDimensions();
                reportInfo() << "TensorRT found input node " << input->getName() << "with shape: " << getTensorShape(dims).toString() << reportEnd();
                dims.d[0] = 1;
                profile->setDimensions(input->getName(), nvinfer1::OptProfileSelector::kMIN, dims);
                profile->setDimensions(input->getName(), nvinfer1::OptProfileSelector::kOPT, dims);
                dims.d[0] = m_maxBatchSize;
                profile->setDimensions(input->getName(), nvinfer1::OptProfileSelector::kMAX, dims);
            }
            config->addOptimizationProfile(profile);
        }

        builder->setMaxBatchSize(m_maxBatchSize);
        m_engine = builder->buildEngineWithConfig(*network, *config);
        if(!m_engine)
            throw Exception("Failed to build CUDA engine for TensorRT");
        reportInfo() << "Finished building CUDA engine for TensorRT" << reportEnd();

        // Make sure serialization folder exists
        createDirectories(Config::getKernelBinaryPath());
        // Serialize the model
        std::unique_ptr<nvinfer1::IHostMemory, decltype(Destroy())> serializedModel(m_engine->serialize(), Destroy());
        // Store model to disk
        std::ofstream ofile(serializedBinaryFilename.c_str(), std::ios::binary);
        ofile.write((char *) serializedModel->data(), serializedModel->size());
        ofile.close();
        std::ofstream ofileCache(serializedCacheFilename.c_str());
        std::string modifiedDate = getModifiedDate(filename) + "\n";
        std::string TensorRTVersion = std::to_string(getInferLibVersion()) + "\n";
        ofileCache.write(modifiedDate.c_str(), modifiedDate.size());
        ofileCache.write(TensorRTVersion.c_str(), TensorRTVersion.size());
        ofileCache.close();
    } else {
        std::unique_ptr<nvinfer1::IRuntime, decltype(Destroy())> runtime(nvinfer1::createInferRuntime(gLogger), Destroy());
        // Read serialized model from disk
        std::ifstream ifile(serializedBinaryFilename.c_str(), std::ios::binary);
        std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(ifile), {});
        ifile.close();
        // Deserialize the model data
        m_engine = runtime->deserializeCudaEngine(buffer.data(), buffer.size(), nullptr);
        if(m_engine == nullptr)
            throw Exception("Unable to deserialize file. You should delete the serialization files " + serializedCacheFilename + " and " + serializedBinaryFilename + " and try again.");
    }

    const bool inputsDefined = !mInputNodes.empty();
    const bool outputsDefined = !mOutputNodes.empty();
    // Get input and output nodes from the CUDA engine
    if(filename.substr(filename.size()-4) != ".uff") {
        int inputCount = 0;
        int outputCount = 0;
        for (int i = 0; i < m_engine->getNbBindings(); ++i) {
            auto name = m_engine->getBindingName(i);
            auto shape = getTensorShape(m_engine->getBindingDimensions(i));
            NodeType type;
            if(shape.getDimensions() >= 4) { // If image; 2D or 3D
                type = NodeType::IMAGE;
                // Try to determine channel ordering since TensorRT API doesn't seem to have a good way to do this..
                if(m_engine->bindingIsInput(i)) {
                    if (shape[shape.getDimensions() - 1] <= 4) {
                        m_imageOrdering = ImageOrdering::ChannelLast;
                        reportInfo() << "Guessed image ordering to be channel last as shape was " << shape.toString()
                        << reportEnd();
                    } else {
                        m_imageOrdering = ImageOrdering::ChannelFirst;
                        reportInfo() << "Guessed image ordering to be channel first as shape was " << shape.toString()
                        << reportEnd();
                    }
                }
            } else {
                type = NodeType::TENSOR;
            }
            if(m_engine->bindingIsInput(i)) {
                reportInfo() << "Found input node " << name << " with shape " << shape.toString() << reportEnd();
                if(inputsDefined) {
                    if(mInputNodes.count(name) > 0) {
                        reportInfo() << "Node was defined by user at id " << mInputNodes[name].portID  << reportEnd();
                        if(mInputNodes[name].shape.empty())
                            mInputNodes[name].shape = shape;
                    } else {
                        reportInfo() << "Ignored input node " << name << " because input nodes were specified, but not this one." << reportEnd();
                    }
                } else {
                    addInputNode(inputCount, name, type, shape);
                    ++inputCount;
                }
            } else {
                reportInfo() << "Found output node " << name << " with shape " << shape.toString() << reportEnd();
                if(outputsDefined) {
                    if(mOutputNodes.count(name) > 0) {
                        reportInfo() << "Node was defined by user at id " << mOutputNodes[name].portID  << reportEnd();
                        if(mOutputNodes[name].shape.empty()) {
                            reportInfo() << "Shape was empty, setting it to " << shape.toString() << reportEnd();
                            mOutputNodes[name].shape = shape;
                        }
                    } else {
                        reportInfo() << "Ignored output node " << name << " because output nodes were specified, but not this one." << reportEnd();
                    }
                } else {
                    addOutputNode(outputCount, name, type, shape);
                    ++outputCount;
                }
            }
        }
    }

    m_context = m_engine->createExecutionContext();
    setIsLoaded(true);
}

ImageOrdering TensorRTEngine::getPreferredImageOrdering() const {
    if(!isLoaded())
        throw Exception("Network must be loaded before calling getPreferredImageOrdering on TensorRTEngine");
    return m_imageOrdering;
}

TensorRTEngine::~TensorRTEngine() {
    for(auto buffer : m_cudaBuffers) {
        CUDA_CHECK(cudaFree(buffer));
    }
    reportInfo() << "Finished freeing TensorRT buffer data" << reportEnd();
    if(m_context != nullptr)
        m_context->destroy();
    if(m_engine != nullptr)
        m_engine->destroy();
}

TensorRTEngine::TensorRTEngine() {

}

std::string TensorRTEngine::getName() const {
    return "TensorRT";
}

void TensorRTEngine::setMaxBatchSize(int maxBatchSize) {
    if(maxBatchSize <= 0)
        throw Exception("Max batch size must be > 0");
    m_maxBatchSize = maxBatchSize;
}

int TensorRTEngine::getMaxBatchSize() const {
    return m_maxBatchSize;
}

}
