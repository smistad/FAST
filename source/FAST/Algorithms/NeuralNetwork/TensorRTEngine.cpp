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
            if (severity != Severity::kINFO)
                std::cout << msg << std::endl;
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
    nvinfer1::IExecutionContext* context = m_engine->createExecutionContext();

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
    auto tensorData = access->getData<3>();
    CHECK(cudaMemcpy(buffers[bindingIdxInput], tensorData.data(), buffersSizes[bindingIdxInput].first * elementSize(buffersSizes[bindingIdxInput].second), cudaMemcpyHostToDevice));

    context->execute(batchSize, &buffers[0]);

    // Free input memory buffers
    CHECK(cudaFree(buffers[bindingIdxInput]));

    // Free output memory buffers
    for(int bindingIdx = 0; bindingIdx < nbBindings; ++bindingIdx)
        if(!m_engine->bindingIsInput(bindingIdx))
            CHECK(cudaFree(buffers[bindingIdx]));

    // TODO transfer output data back
    auto outputData = make_uninitialized_unique<float[]>(buffersSizes[bindingIdxOutput].first);
    CHECK(cudaMemcpy(buffers[bindingIdxOutput], outputData.get(), buffersSizes[bindingIdxOutput].first * elementSize(buffersSizes[bindingIdxOutput].second), cudaMemcpyDeviceToHost));

    auto outputTensor = Tensor::New();
    mOutputNodes.begin()->second.data = outputTensor;
    outputTensor->create(std::move(outputData), mOutputNodes.begin()->second.shape);

    context->destroy();

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
        if(shape.getDimensions() == 4)
            parser->registerInput(node.first.c_str(), nvinfer1::Dims3(shape[1], shape[2], shape[3]), nvuffparser::UffInputOrder::kNHWC);
        if(shape.getDimensions() == 5)
            parser->registerInput(node.first.c_str(), nvinfer1::Dims4(shape[1], shape[2], shape[3], shape[4]), nvuffparser::UffInputOrder::kNHWC);
    }
    reportInfo() << "Input nodes setup" << reportEnd();

    // Setup output nodes
    for(auto node : mOutputNodes) {
        parser->registerOutput(node.first.c_str());
    }
    reportInfo() << "Output nodes setup" << reportEnd();

    if(!parser->parse(filename.c_str(), *network, nvinfer1::DataType::kFLOAT))
        throw Exception("Error parsing UFF file " + filename);


    builder->setMaxBatchSize(m_maxBatchSize);
    builder->setMaxWorkspaceSize(m_maxWorkspaceSize);

    m_engine = builder->buildCudaEngine(*network);
    if(!m_engine)
        throw Exception("Failed to build CUDA engine for TensorRT");
}

ImageOrdering TensorRTEngine::getPreferredImageOrdering() const {
    //return ImageOrdering::CWH;
    return ImageOrdering::HWC;
}

TensorRTEngine::~TensorRTEngine() {
    if(m_engine != nullptr)
        m_engine->destroy();
    nvuffparser::shutdownProtobufLibrary();
}

TensorRTEngine::TensorRTEngine() {

}

}