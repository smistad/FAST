#include "TensorRTEngine.hpp"
#include "NvInfer.h"
#include "NvUffParser.h"
#include "NvUtils.h"
#include <cuda_runtime_api.h>
#include <FAST/Utility.hpp>

#define CUDA_CHECK(status)                             \
    do                                            \
    {                                             \
        auto ret = (status);                      \
        if (ret != 0)                             \
        {                                         \
            std::cout << "Cuda failure: " << ret; \
            abort();                              \
        }                                         \
    } while (0)


namespace fast {

/**
 * Logger object for TensorRT
 */
class Logger : public nvinfer1::ILogger {
    void log(Severity severity, const char* msg) override {
        switch(severity) {
            case Severity::kINFO:
                Reporter::info() << msg << Reporter::end();
                break;
            case Severity::kERROR:
            case Severity::kINTERNAL_ERROR:
                Reporter::error() << msg << Reporter::end();
                break;
            case Severity::kWARNING:
                Reporter::warning() << msg << Reporter::end();
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


inline int64_t volume(const nvinfer1::Dims& d) {
    int64_t v = 1;
    for (int64_t i = 0; i < d.nbDims; i++)
        v *= d.d[i];
    return v;
}


static std::vector<std::pair<int64_t, nvinfer1::DataType>>
calculateBindingBufferSizes(const nvinfer1::ICudaEngine& engine, int nbBindings, int batchSize) {
    std::vector<std::pair<int64_t, nvinfer1::DataType>> sizes;
    for (int i = 0; i < nbBindings; ++i)
    {
        nvinfer1::Dims dims = engine.getBindingDimensions(i);
        nvinfer1::DataType dtype = engine.getBindingDataType(i);

        int64_t eltCount = volume(dims) * batchSize;
        sizes.push_back(std::make_pair(eltCount, dtype));
    }

    return sizes;
}

void* safeCudaMalloc(size_t memSize)
{
    void* deviceMem;
    CUDA_CHECK(cudaMalloc(&deviceMem, memSize));
    if (deviceMem == nullptr)
    {
        std::cerr << "Out of memory" << std::endl;
        exit(1);
    }
    return deviceMem;
}



void TensorRTEngine::run() {

    const int nbBindings = m_engine->getNbBindings();

    // Get batch size
    const int batchSize = mInputNodes.begin()->second.data->getShape()[0];
    if(batchSize > m_engine->getMaxBatchSize()) {
        reportWarning() << "Batch is larger than the max batch size given to TensorRT" << reportEnd();
    }

    std::vector<void*> buffers(nbBindings);
    auto buffersSizes = calculateBindingBufferSizes(*m_engine, nbBindings, batchSize);

    // TODO assuming 1 input and output here
    std::map<std::string, int> inputIndexes;
    std::map<std::string, int> outputIndexes;
    for(int i = 0; i < nbBindings; ++i) {
        auto name = m_engine->getBindingName(i);
        if (m_engine->bindingIsInput(i)) {
            inputIndexes[name] = i;
        } else {
            outputIndexes[name] = i;
        }
        // Allocate data
        buffers[i] = safeCudaMalloc(buffersSizes[i].first * elementSize(buffersSizes[i].second));
    }

    // Allocate data for each input and copy data to it
    for(const auto& inputNode : mInputNodes) {
        auto tensor = inputNode.second.data;
        auto access = tensor->getAccess(ACCESS_READ);
        float* tensorData;
        switch(tensor->getShape().getDimensions()) {
            case 2:
                tensorData = access->getData<2>().data();
                break;
            case 3:
                tensorData = access->getData<3>().data();
                break;
            case 4:
                tensorData = access->getData<4>().data();
                break;
            case 5:
                tensorData = access->getData<5>().data();
                break;
			case 6:
                tensorData = access->getData<6>().data();
				break;
            default:
                throw Exception("Invalid tensor dimension size");
		}
        const int index = inputIndexes.at(inputNode.first);
        CUDA_CHECK(cudaMemcpy(buffers[index], tensorData,
                              buffersSizes[index].first * elementSize(buffersSizes[index].second),
                              cudaMemcpyHostToDevice));
        reportInfo() << "Finished copying input data to TensorRT" << reportEnd();
    }

    // Execute network
    m_context->execute(batchSize, &buffers[0]);
    reportInfo() << "Finished execute TensorRT" << reportEnd();

    // Free input memory buffers
    for(const auto& input : inputIndexes)
        CUDA_CHECK(cudaFree(buffers[input.second]));
    reportInfo() << "Finished freeing input data TensorRT" << reportEnd();

    // Transfer output data back
    for(const auto& output : outputIndexes) {
        const int index = output.second;
        reportInfo() << "Processing output node " << output.first << reportEnd();
        auto outputData = make_uninitialized_unique<float[]>(buffersSizes[index].first);
        CUDA_CHECK(cudaMemcpy(outputData.get(), buffers[index],
                              buffersSizes[index].first * elementSize(buffersSizes[index].second),
                              cudaMemcpyDeviceToHost));
        auto outputTensor = Tensor::New();
        mOutputNodes.at(output.first).data = outputTensor;

        // Get output shape
        nvinfer1::Dims dims = m_engine->getBindingDimensions(index);
        TensorShape shape({batchSize});
        for(int i = 0; i < dims.nbDims; ++i)
            shape.addDimension(dims.d[i]);
        // Remove last dimensions which have value 1
        int lastIndex = shape.getDimensions()-1;
        for(int i = shape.getDimensions()-1; i >= 0; --i) {
            if(shape[i] != 1) {
                lastIndex = i+1;
                break;
            }
        }
        shape.deleteDimensions(lastIndex, shape.getDimensions());
        reportInfo() << "Output shape inferred to be: " << shape.toString() << reportEnd();

        outputTensor->create(std::move(outputData), shape);
        reportInfo() << "Finished moving data to FAST tensor, TensorRT" << reportEnd();
        reportInfo() << "Finished transfer of output data TensorRT" << reportEnd();
        CUDA_CHECK(cudaFree(buffers[index]));
        reportInfo() << "Finished freeing output data TensorRT" << reportEnd();
    }
}

void TensorRTEngine::load() {

    // Check that input and output nodes are set
    if(mInputNodes.empty() || mOutputNodes.empty())
        throw Exception("Input and output nodes must be defined before loading Uff files using the TensorRT engine");

    const auto filename = getFilename();
    if(!fileExists(filename))
        throw FileNotFoundException(filename);
    reportInfo() << "Loading file " << filename << " using TensorRT" << reportEnd();
    std::unique_ptr<nvinfer1::IBuilder, decltype(Destroy())> builder(nvinfer1::createInferBuilder(gLogger), Destroy());
    std::unique_ptr<nvinfer1::INetworkDefinition, decltype(Destroy())> network(builder->createNetwork(), Destroy());
    std::unique_ptr<nvuffparser::IUffParser, decltype(Destroy())> parser(nvuffparser::createUffParser(), Destroy());
    reportInfo() << "Created builders and parsers" << reportEnd();

    // Setup input nodes
    reportInfo() << "Going through nodes.." << reportEnd();
    for(auto node : mInputNodes) {
        reportInfo() << node.first << reportEnd();
        auto shape = node.second.shape;
        if(shape.getDimensions() == 0)
            throw Exception("Unknown shape for input node " + node.first + ". For TensorRT you need to specify the input tensor shape explictly with addInptNode");
        if(shape.getDimensions() == 4)
            parser->registerInput(node.first.c_str(), nvinfer1::Dims3(shape[1], shape[2], shape[3]), nvuffparser::UffInputOrder::kNCHW);
        if(shape.getDimensions() == 5)
            parser->registerInput(node.first.c_str(), nvinfer1::Dims4(shape[1], shape[2], shape[3], shape[4]), nvuffparser::UffInputOrder::kNCHW);
    }
    reportInfo() << "Input nodes finished" << reportEnd();

    // Setup output nodes
    for(auto node : mOutputNodes) {
        parser->registerOutput(node.first.c_str());
    }
    reportInfo() << "Output nodes finished" << reportEnd();

    if(!parser->parse(filename.c_str(), *network, nvinfer1::DataType::kFLOAT))
        throw Exception("Error parsing UFF file " + filename);

    reportInfo() << "Finished parsing UFF file" << reportEnd();

    builder->setMaxBatchSize(m_maxBatchSize);
    builder->setMaxWorkspaceSize(m_maxWorkspaceSize);
    //builder->setFp16Mode(builder->platformHasFastFp16());

    m_engine = builder->buildCudaEngine(*network);
    if(!m_engine)
        throw Exception("Failed to build CUDA engine for TensorRT");
    reportInfo() << "Finished building CUDA engine for TensorRT" << reportEnd();

    m_context = m_engine->createExecutionContext();
    setIsLoaded(true);
}

ImageOrdering TensorRTEngine::getPreferredImageOrdering() const {
    return ImageOrdering::CHW;
}

TensorRTEngine::~TensorRTEngine() {
    if(m_engine != nullptr)
        m_engine->destroy();
    if(m_context != nullptr)
        m_context->destroy();
    //nvuffparser::shutdownProtobufLibrary(); // This cannot be called twice in the same program. Maybe use atexit instead?
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

std::string TensorRTEngine::getDefaultFileExtension() const {
    return "uff";
}

}