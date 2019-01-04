#include "TensorRTEngine.hpp"
#include "NvInfer.h"
#include "NvUffParser.h"
#include "NvUtils.h"
#include <cuda_runtime_api.h>
#include <FAST/Utility.hpp>

#define CHECK(status)                             \
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

    class Logger : public nvinfer1::ILogger {
        void log(Severity severity, const char* msg) override {
            // suppress info-level messages
            //if(severity != Severity::kINFO)
            Reporter::info() << msg << Reporter::end();
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
    CHECK(cudaMalloc(&deviceMem, memSize));
    if (deviceMem == nullptr)
    {
        std::cerr << "Out of memory" << std::endl;
        exit(1);
    }
    return deviceMem;
}



void TensorRTEngine::run() {

    int batchSize = 1;

    int nbBindings = m_engine->getNbBindings();
    assert(nbBindings == 2);

    std::vector<void*> buffers(nbBindings);
    auto buffersSizes = calculateBindingBufferSizes(*m_engine, nbBindings, batchSize);

    // TODO assuming 1 input and output here
    int bindingIdxInput = 0;
    int bindingIdxOutput = 0;
    for (int i = 0; i < nbBindings; ++i)
    {
        if (m_engine->bindingIsInput(i)) {
            bindingIdxInput = i;
        } else {
            bindingIdxOutput = i;
        }
        // Allocate data
        buffers[i] = safeCudaMalloc(buffersSizes[i].first * elementSize(buffersSizes[i].second));
    }

    // Allocate data for input and copy data to it
    auto tensor = mInputNodes.begin()->second.data;
    auto access = tensor->getAccess(ACCESS_READ);
    std::cout << tensor->getShape().toString() << std::endl;
    auto tensorData = access->getData<4>();
    CHECK(cudaMemcpy(buffers[bindingIdxInput], tensorData.data(), buffersSizes[bindingIdxInput].first * elementSize(buffersSizes[bindingIdxInput].second), cudaMemcpyHostToDevice));
    reportInfo() << "Finished copying input data to TensorRT" << reportEnd();

    // Execute network
    m_context->execute(batchSize, &buffers[0]);
    reportInfo() << "Finished execute TensorRT" << reportEnd();

    // Free input memory buffers
    CHECK(cudaFree(buffers[bindingIdxInput]));
    reportInfo() << "Finished freeing input data TensorRT" << reportEnd();

    // Transfer output data back
    auto outputData = make_uninitialized_unique<float[]>(buffersSizes[bindingIdxOutput].first);
    CHECK(cudaMemcpy(outputData.get(), buffers[bindingIdxOutput],  buffersSizes[bindingIdxOutput].first * elementSize(buffersSizes[bindingIdxOutput].second), cudaMemcpyDeviceToHost));
    reportInfo() << "Finished transfer of output data TensorRT" << reportEnd();

    // Free output memory buffers
    for(int bindingIdx = 0; bindingIdx < nbBindings; ++bindingIdx)
        if(!m_engine->bindingIsInput(bindingIdx))
            CHECK(cudaFree(buffers[bindingIdx]));
    reportInfo() << "Finished freeing output data TensorRT" << reportEnd();

    auto outputTensor = Tensor::New();
    mOutputNodes.begin()->second.data = outputTensor;

    // Get output shape
    nvinfer1::Dims dims = m_engine->getBindingDimensions(bindingIdxOutput);
    TensorShape shape({1, dims.d[0], dims.d[1], dims.d[2]});
    reportInfo() << "Output shape inferred to be: " << shape.toString() << reportEnd();

    outputTensor->create(std::move(outputData), shape);
    reportInfo() << "Finished moving data to FAST tensor, TensorRT" << reportEnd();
}

void TensorRTEngine::load() {

    // Check that input and output nodes are set
    if(mInputNodes.empty() || mOutputNodes.empty())
        throw Exception("Input and output nodes must be defined before loading Uff files using the TensorRT engine");

    const auto filename = getFilename();
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
        std::cout << shape.toString() << std::endl;
        //if(shape.getDimensions() == 4)
            parser->registerInput(node.first.c_str(), nvinfer1::Dims3(1, 256, 256), nvuffparser::UffInputOrder::kNCHW);
        //if(shape.getDimensions() == 5)
        //    parser->registerInput(node.first.c_str(), nvinfer1::Dims4(shape[1], shape[2], shape[3], shape[4]), nvuffparser::UffInputOrder::kNCHW);
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
    nvuffparser::shutdownProtobufLibrary();
}

TensorRTEngine::TensorRTEngine() {

}

}