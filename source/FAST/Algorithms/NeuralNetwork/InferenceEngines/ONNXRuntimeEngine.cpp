#include "ONNXRuntimeEngine.hpp"
#include <onnxruntime_cxx_api.h>
#ifdef WIN32
#include <dml_provider_factory.h>
#elif defined(__APPLE__) || defined(__MACOSX)
#include <coreml_provider_factory.h>
#else
#endif

#include <FAST/Config.hpp>

namespace fast {

// pretty prints a shape dimension vector
std::string print_shape(const std::vector<int64_t>& v) {
  std::stringstream ss("");
  for (size_t i = 0; i < v.size() - 1; i++)
    ss << v[i] << "x";
  ss << v[v.size() - 1];
  return ss.str();
}


void ONNXRuntimeEngine::run() {
	//auto start = std::chrono::high_resolution_clock::now();
    std::vector<const char*> inputNames;
    std::vector<Ort::Value> inputTensors;
	// Important to use reference here, as we are using c_str() which does not copy string, and will be deleted if std::string is deleted.
    for (const auto& inputNode : mInputNodes) {
        auto tensor = inputNode.second.data;
        auto access = tensor->getAccess(ACCESS_READ);
		inputNames.push_back(strdup(inputNode.first.c_str()));
        float* tensorData = access->getRawData();
        reportInfo() << "ONNXRuntime: Creating memory info.." << reportEnd();
        Ort::MemoryInfo info = Ort::MemoryInfo::CreateCpu(OrtAllocatorType::OrtArenaAllocator, OrtMemType::OrtMemTypeCPU); // Must be TypeCPU to work on CPU
        auto shape = tensor->getShape();

        std::vector<int64_t> dims;
        for (int x : shape.getAll())
            dims.push_back(x);
        inputTensors.emplace_back(Ort::Value::CreateTensor<float>(info, tensorData, shape.getTotalSize(), dims.data(), shape.getDimensions()));
    }
    std::vector<const char*> outputNames;
	// Important to use reference here, as we are using c_str() which does not copy string, and will be deleted if std::string is deleted.
    for (const auto& outputNode : mOutputNodes) {
        outputNames.push_back(outputNode.first.c_str());
    }
		
    Ort::RunOptions runOptions;
    reportInfo() << "Running ONNX runtime .." << reportEnd();
    std::vector<Ort::Value> output = m_session->Run(runOptions, inputNames.data(), inputTensors.data(), inputNames.size(), outputNames.data(), outputNames.size());
    reportInfo() << "Finished run ONNX runtime" << reportEnd();
    //std::chrono::duration<float, std::milli> duration = std::chrono::high_resolution_clock::now() - start;
    //std::cout << "Run: " << duration.count() << std::endl;

    // Copy output data to FAST
    int counter = 0;
    for(auto& outputNode : mOutputNodes) {
        const float* data = output[counter].GetTensorData<float>();
        const auto name = outputNode.first;
        auto shape2 = outputNode.second.shape;
        shape2[0] = 1;
        auto outputTensor = Tensor::create(data, shape2);
        outputNode.second.data = outputTensor;
        ++counter;
    }
}

void ONNXRuntimeEngine::load() {
    const auto filename = getFilename();
    std::wstring wideStr(filename.begin(), filename.end());

	reportInfo() << "Setting up ONNX Runtime" << reportEnd();
    //auto start = std::chrono::high_resolution_clock::now();
	m_env = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "ONNXRuntime");
    if(m_deviceType == InferenceDeviceType::CPU) {
		Ort::SessionOptions session_options;
#ifdef WIN32
		m_session = std::make_unique<Ort::Session>(*m_env.get(), wideStr.c_str(), session_options);
#else
		m_session = std::make_unique<Ort::Session>(*m_env.get(), filename.c_str(), session_options);
#endif
    } else {
#ifdef WIN32
        try {
            Ort::SessionOptions session_options;
            SetDllDirectory(Config::getLibraryPath().c_str()); // Make sure delay-load dlls are found (directml.dll) etc.
            Ort::ThrowOnError(OrtSessionOptionsAppendExecutionProvider_DML(session_options, 0));
            session_options.DisableMemPattern();
            m_session = std::make_unique<Ort::Session>(*m_env.get(), wideStr.c_str(), session_options);
            SetDllDirectory("");
        }
        catch (Ort::Exception& e) {
            reportWarning() << "Exception occured while trying to load DirectML for ONNXRuntime with message: (" << e.GetOrtErrorCode() << ") " << e.what()  << ". Falling back to CPU." << reportEnd();
            Ort::SessionOptions session_options;
            m_session = std::make_unique<Ort::Session>(*m_env.get(), wideStr.c_str(), session_options);
        }
#elif defined(__APPLE__) || defined(__MACOSX)
        // APPLE
        try {
            Ort::SessionOptions session_options;
            uint32_t coreml_flags = 0;
            Ort::ThrowOnError(OrtSessionOptionsAppendExecutionProvider_CoreML(session_options, coreml_flags));
            m_session = std::make_unique<Ort::Session>(*m_env.get(), filename.c_str(), session_options);
        }
        catch (Ort::Exception& e) {
            reportWarning() << "Exception occured while trying to load CoreML for ONNXRuntime with message: (" << e.GetOrtErrorCode() << ") " << e.what() << ". Falling back to CPU." << reportEnd();
            Ort::SessionOptions session_options;
            m_session = std::make_unique<Ort::Session>(*m_env.get(), filename.c_str(), session_options);
        }
#else
        // LINUX
#endif
    }
	Ort::AllocatorWithDefaultOptions allocator;
	reportInfo() << "ONNXRuntime Session created" << reportEnd();

    //std::chrono::duration<float, std::milli> duration = std::chrono::high_resolution_clock::now() - start;
    //std::cout << "Setup: " << duration.count() << std::endl;

    // Parse input and output nodes
    const bool inputsDefined = !mInputNodes.empty();
    const bool outputsDefined = !mOutputNodes.empty();
    int inputCount = 0;
    int outputCount = 0;
	for (size_t i = 0; i < m_session->GetInputCount(); i++) {
        std::string name = m_session->GetInputNameAllocated(i, allocator).get();
		reportInfo() << "Found input node: " << name << " : " << print_shape(m_session->GetInputTypeInfo(i).GetTensorTypeAndShapeInfo().GetShape()) << reportEnd();
		auto shape = TensorShape();
		for(int x : m_session->GetInputTypeInfo(i).GetTensorTypeAndShapeInfo().GetShape()) {
			shape.addDimension(x);
		}

		NodeType type;
        if (shape.getDimensions() >= 4) { // If image; 2D or 3D
            type = NodeType::IMAGE;
		   if(shape[shape.getDimensions() - 1] <= 4) {
				m_imageOrdering = ImageOrdering::ChannelLast;
				reportInfo() << "Guessed image ordering to be channel last as shape was " << shape.toString() << reportEnd();
			} else {
				m_imageOrdering = ImageOrdering::ChannelFirst;
				reportInfo() << "Guessed image ordering to be channel first as shape was " << shape.toString() << reportEnd();
			}
        } else {
			type = NodeType::TENSOR;
        }
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
	}

	for (size_t i = 0; i < m_session->GetOutputCount(); i++) {
        std::string name = m_session->GetOutputNameAllocated(i, allocator).get();
		reportInfo() << "Found output node: " << name << " : " << print_shape(m_session->GetOutputTypeInfo(i).GetTensorTypeAndShapeInfo().GetShape()) << reportEnd();
		auto shape = TensorShape();
		for (int x : m_session->GetOutputTypeInfo(i).GetTensorTypeAndShapeInfo().GetShape()) {
			shape.addDimension(x);
		}
		NodeType type;
        if (shape.getDimensions() >= 4) { // If image; 2D or 3D
            type = NodeType::IMAGE;
        } else {
            type = NodeType::TENSOR;
        }
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
  setIsLoaded(true);
}

ImageOrdering ONNXRuntimeEngine::getPreferredImageOrdering() const {
    if(!isLoaded())
        throw Exception("Network must be loaded before calling getPreferredImageOrdering on ONNXRuntimeEngine");
    return m_imageOrdering;
}

ONNXRuntimeEngine::~ONNXRuntimeEngine() {

}

ONNXRuntimeEngine::ONNXRuntimeEngine() {

}

std::vector<InferenceDeviceInfo> ONNXRuntimeEngine::getDeviceList() {
    // TODO improve this logic
    std::vector<InferenceDeviceInfo> result;
    InferenceDeviceInfo cpu;
    cpu.type = InferenceDeviceType::CPU;
    cpu.index = 0;
    result.push_back(cpu);
    InferenceDeviceInfo gpu;
    gpu.type = InferenceDeviceType::GPU;
    gpu.index = 0;
    result.push_back(gpu);
    return result;
}

std::string ONNXRuntimeEngine::getName() const {
    return "ONNXRuntime";
}

void ONNXRuntimeEngine::setMaxBatchSize(int maxBatchSize) {
    if(maxBatchSize <= 0)
        throw Exception("Max batch size must be > 0");
    m_maxBatchSize = maxBatchSize;
}

int ONNXRuntimeEngine::getMaxBatchSize() const {
    return m_maxBatchSize;
}

}
