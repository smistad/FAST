#include "OpenVINOEngine.hpp"
#include <openvino/openvino.hpp>
#include <FAST/Utility.hpp>

namespace fast {

class OpenVINOInfer {
public:
    ov::InferRequest request;
};


void OpenVINOEngine::run() {
    std::lock_guard<std::mutex> lock(m_mutex);

    // Copy input data
    reportInfo() << "OpenVINO processing input nodes." << reportEnd();
    for(auto inputNode : mInputNodes) {
        const auto index = m_inputIndices[inputNode.first];
        auto tensor = inputNode.second.data;
        auto access = tensor->getAccess(ACCESS_READ);
        float* tensorData = access->getRawData();
        ov::Shape shape;
        for(int x : tensor->getShape().getAll()) {
            shape.push_back(x);
        }
        m_infer->request.set_input_tensor(index, ov::Tensor(ov::element::f32, shape, tensorData));
    }
    reportInfo() << "OpenVINO input data added." << reportEnd();

    // Run inference
    m_infer->request.infer();
    reportInfo() << "OpenVINO inference done." << reportEnd();

    // Get output data
    reportInfo() << "OpenVINO processing output nodes." << reportEnd();
    for(auto& outputNode : mOutputNodes) {
        const auto index = m_outputIndices[outputNode.first];
        ov::Tensor ovTensor = m_infer->request.get_output_tensor(index);
        const float* data = ovTensor.data<float>();
        // Get shape of output tensor
        auto shape = TensorShape();
        for(int x : ovTensor.get_shape()) {
            shape.addDimension(x);
        }
        const auto name = outputNode.first;
        outputNode.second.shape = shape;
        auto outputTensor = Tensor::create(data, shape);
        outputNode.second.data = outputTensor;
    }
    reportInfo() << "OpenVINO processing output nodes done." << reportEnd();
}

void OpenVINOEngine::load() {
    if(!m_core)
        m_core = std::make_shared<ov::Core>();

    try {
        for(auto device : m_core->get_available_devices()) {
            reportInfo() << "OpenVINO found device: " << device << reportEnd();
        }

        std::shared_ptr<ov::Model> model = m_core->read_model(getFilename());
        reportInfo() << "OpenVINO loaded model" << reportEnd();

        // Go through input and output nodes
        const bool inputsDefined = !mInputNodes.empty();
        const bool outputsDefined = !mOutputNodes.empty();
        std::map<std::string, ov::PartialShape> reshapeMap;
        int inputCount = 0;
        int outputCount = 0;
        bool dynamicInputShapes = false;
        int index = 0;
        bool imageOrderingFoundOnInput = false;
        // Sort nodes for consistency
        std::map<std::string, std::pair<int, ov::Output<ov::Node>>> inputNodes;
        for(auto inputNode : model->inputs()) {
            auto name = inputNode.get_any_name();
            inputNodes[name] = {index, inputNode};
            ++index;
        }
        for(auto node : inputNodes) {
            auto name = node.first;
            auto inputNode = node.second.second;
            reportInfo() << "Found input node " << inputNode.get_any_name() << " with shape " << inputNode.get_partial_shape().to_string() << " and index " << inputNode.get_index() << reportEnd();
            if(inputNode.get_partial_shape().is_dynamic()) {
                dynamicInputShapes = true;
            }
            auto shape = TensorShape();
            for(auto x : inputNode.get_partial_shape()) {
                if(x.is_dynamic()) {
                    shape.addDimension(-1);
                } else {
                    shape.addDimension(x.get_length());
                }
            }
            m_inputIndices[name] = node.second.first;// inputNode.get_index(); // get_index always return 0 for some reason?
            ++index;
            NodeType type = detectNodeType(shape);
            if(type == NodeType::IMAGE) {
                try {
                    m_imageOrdering = detectImageOrdering(shape);
                    imageOrderingFoundOnInput = true;
                } catch(Exception &e) {
                    // Unable to find ordering, try on output instead.
                }
            }
            if(inputsDefined) {
                if(mInputNodes.count(name) > 0) {
                    reportInfo() << "Node was defined by user at id " << mInputNodes[name].id  << reportEnd();
                    if(mInputNodes[name].shape.empty())
                        mInputNodes[name].shape = shape;
                    auto node = mInputNodes[name];
                    if(dynamicInputShapes) {
                        if(!node.minShape.empty() && node.minShape.getUnknownDimensions() == 0 &&
                        !node.maxShape.empty() && node.maxShape.getUnknownDimensions() == 0) {
                            reportInfo() << "Dynamic input shape detected, and shape bounds was given." << reportEnd();
                            std::vector<ov::Dimension> dims;
                            for(int i = 0; i < node.minShape.getDimensions(); ++i) {
                                dims.push_back(ov::Dimension(node.minShape[i], node.maxShape[i]));
                            }
                            reshapeMap[name] = ov::PartialShape(dims);
                        } else if(!node.shape.empty()) {
                            reportInfo() << "Dynamic input shape detected, and specific shape info was given." << reportEnd();
                            std::vector<ov::Dimension> dims;
                            for(int i = 0; i < node.shape.getDimensions(); ++i) {
                                dims.push_back(ov::Dimension(node.shape[i]));
                            }
                            reshapeMap[name] = ov::PartialShape(dims);
                        } else {
                            reportInfo() << "Dynamic input shape detected, but no shape info was given." << reportEnd();
                        }
                    }

                } else {
                    reportInfo() << "Ignored input node " << name << " because input nodes were specified, but not this one." << reportEnd();
                }
            } else {
                if(shape[0] > 0 && shape[0] < m_maxBatchSize) {
                    // If batch dim is specified and differs from m_maxBatchSize, then we need to reshape
                    std::vector<ov::Dimension> dims;
                    dims.push_back(ov::Dimension(1, m_maxBatchSize));
                    for(int i = 1; i < shape.getDimensions(); ++i) {
                        dims.push_back(ov::Dimension(shape[i]));
                    }
                    reshapeMap[name] = ov::PartialShape(dims);
                }
                addInputNode(NeuralNetworkNode(name, type, shape, inputCount));
                ++inputCount;
            }
        }
        // Go through output nodes
        // Sort nodes for consistency
        std::map<std::string, std::pair<int, ov::Output<ov::Node>>> outputNodes;
        index = 0;
        for(auto outputNode : model->outputs()) {
            auto name = outputNode.get_any_name();
            outputNodes[name] = {index, outputNode};
            ++index;
        }
        for(auto node : outputNodes) {
            auto name = node.first;
            auto outputNode = node.second.second;
            reportInfo() << "Found output node " << name << " with shape " << outputNode.get_partial_shape().to_string() << reportEnd();
            auto shape = TensorShape();
            for(auto x : outputNode.get_partial_shape()) {
                if(x.is_dynamic()) {
                    shape.addDimension(-1);
                } else {
                    shape.addDimension(x.get_length());
                }
            }
            m_outputIndices[name] = node.second.first;// outputNode.get_index(); // get_index always returns 0 for some reason..
            NodeType type = detectNodeType(shape);
            if(!imageOrderingFoundOnInput && type == NodeType::IMAGE)
                m_imageOrdering = detectImageOrdering(shape);
            if(outputsDefined) {
                if(mOutputNodes.count(name) > 0) {
                    reportInfo() << "Node was defined by user at id " << mOutputNodes[name].id  << reportEnd();
                    if(mOutputNodes[name].shape.empty())
                        mOutputNodes[name].shape = shape;
                } else {
                    reportInfo() << "Ignored output node " << name << " because output nodes were specified, but not this one." << reportEnd();
                }
            } else {
                addOutputNode(NeuralNetworkNode(name, type, shape, outputCount));
                ++outputCount;
            }
        }

        // Reshape if needed
        if(!reshapeMap.empty())
            model->reshape(reshapeMap);

        // Compile model for a given device
        std::map<InferenceDeviceType, std::string> deviceMap = {
                {InferenceDeviceType::ANY, "AUTO"},
                {InferenceDeviceType::GPU, "AUTO:GPU"},
                {InferenceDeviceType::CPU, "AUTO:CPU"},
                {InferenceDeviceType::VPU, "AUTO:MYRIAD"},
                };
        ov::CompiledModel compiled_model = m_core->compile_model(model, deviceMap[m_deviceType]);
        reportInfo() << "OpenVINO successfully compiled model" << reportEnd();
        ov::Any execution_devices = compiled_model.get_property(ov::execution_devices);
        reportInfo() << "OpenVINO is running network on " << execution_devices->to_string() << reportEnd();

        // Create infer request
        m_infer = std::make_shared<OpenVINOInfer>();
        m_infer->request = compiled_model.create_infer_request();

        setIsLoaded(true);
    } catch(ov::Exception &e) {
        throw Exception("OpenVINO exception caught: " + std::string(e.what()));
    }
}

std::vector<InferenceDeviceInfo> OpenVINOEngine::getDeviceList() {
    if(!m_core)
        m_core = std::make_shared<ov::Core>();
    std::vector<InferenceDeviceInfo> result;
    for(auto device : m_core->get_available_devices()) {
        InferenceDeviceInfo fastDeviceInfo;
        if(device.substr(0, 3) == "CPU") {
            fastDeviceInfo.type = InferenceDeviceType::CPU;
        } else if(device.substr(0,3) == "GPU"){
            fastDeviceInfo.type = InferenceDeviceType::GPU;
        } else if(device.substr(0,6) == "MYRIAD") {
            fastDeviceInfo.type = InferenceDeviceType::VPU;
        } else {
            continue;
        }
        fastDeviceInfo.index = 0;
        if(device.find('.') != std::string::npos) {
            fastDeviceInfo.index = std::stoi(device.substr(device.find('.')+1));
        }
        result.push_back(fastDeviceInfo);
    }

    return result;
}

ImageOrdering OpenVINOEngine::getPreferredImageOrdering() const {
    if(!isLoaded())
        throw Exception("Network must be loaded before calling getPreferredImageOrdering on TensorRTEngine");

    return m_imageOrdering;
}

std::string OpenVINOEngine::getName() const {
    return "OpenVINO";
}

OpenVINOEngine::~OpenVINOEngine() {
    //if(m_inferState != nullptr)
    //    delete m_inferState;
}

void OpenVINOEngine::loadCustomPlugins(std::vector<std::string> filenames) {
    /*
    if(isLoaded())
        throw Exception("Call loadCustomPlugins before load()");

    if(!m_inferenceCore)
        m_inferenceCore = std::make_shared<Core>();

    for(auto&& filename : filenames) {
        if(!fileExists(filename))
            throw FileNotFoundException(filename);
        if(stringToLower(filename.substr(filename.length()-4)) == "xml") {
            m_inferenceCore->SetConfig({ { ::InferenceEngine::PluginConfigParams::KEY_CONFIG_FILE, filename} }, "GPU"); // TODO how to detect VPU?
        } else {
            auto extension_ptr = make_so_pointer<::InferenceEngine::IExtension>(filename);
            m_inferenceCore->AddExtension(extension_ptr, "CPU");
        }
    }*/
}

}
