#include "TensorFlowEngine.hpp"

// Windows hack for removing need for protobuf
#ifdef WIN32
#include <google/protobuf/stubs/logging.h>
#undef GOOGLE_LOG_IF
#define GOOGLE_LOG_IF(LEVEL, CONDITION) \
  !(CONDITION) ? std::clog : std::cerr
// end hack
#endif

//#include <tensorflow/core/framework/step_stats.pb.h>
//#include <tensorflow/core/framework/tensor.h>
//#include <tensorflow/core/framework/types.pb.h>
//#include <tensorflow/core/lib/strings/stringprintf.h>
//#include <tensorflow/core/platform/env.h>
//#include <tensorflow/core/platform/mutex.h>
//#include <tensorflow/core/platform/types.h>
#include <tensorflow/core/public/session.h>
//#include <tensorflow/core/graph/default_device.h>
//#include <tensorflow/core/platform/init_main.h>
//#include <tensorflow/cc/framework/ops.h>
//#include <tensorflow/core/platform/logging.h>
#include <FAST/Utility.hpp>
#include <tensorflow/cc/saved_model/loader.h>
#include <tensorflow/cc/saved_model/loader_util.h>
#include <tensorflow/cc/saved_model/tag_constants.h>
#include <tensorflow/cc/saved_model/reader.h>
#include <tensorflow/c/c_api.h>
#include <tensorflow/c/tf_status.h>


namespace fast {


class TensorFlowTensorWrapper {
    public:
        explicit TensorFlowTensorWrapper(tensorflow::Tensor&& inTensor) : tensor(inTensor) {};
        tensorflow::Tensor tensor;
};

TensorFlowTensor::TensorFlowTensor(TensorFlowTensorWrapper* wrapper) {
    m_tensorflowTensor = wrapper;
    auto shape = m_tensorflowTensor->tensor.shape();
    TensorShape fastShape;
    for(int i = 0; i < shape.dims(); ++i)
        fastShape.addDimension(shape.dim_size(i));

    m_shape = fastShape;
    m_spacing = VectorXf::Ones(m_shape.getDimensions());
    mHostDataIsUpToDate = true;
    if(m_shape.getDimensions() >= 3) {
        const int width = m_shape[m_shape.getDimensions() - 2];
        const int height = m_shape[m_shape.getDimensions() - 3];
        mBoundingBox = DataBoundingBox(Vector3f(width, height, 1));
    }
    /*
    // Unnecessary copy..
    auto data = make_uninitialized_unique<float[]>(fastShape.getTotalSize());
    std::memcpy(data.get(), tensorflowTensor.flat<float>().data(), fastShape.getTotalSize()*sizeof(float));
    Tensor::create(std::move(data), fastShape);
     */
}

bool TensorFlowTensor::hasAnyData() {
    return true;
}

TensorFlowTensor::~TensorFlowTensor() {
    delete m_tensorflowTensor;
}

float* TensorFlowTensor::getHostDataPointer() {
    return m_tensorflowTensor->tensor.flat<float>().data();
}

static TensorShape getShape(const tensorflow::NodeDef& node) {
    TensorShape resultShape;
    if(node.attr().count("shape") > 0) {
        auto shape = node.attr().at("shape").shape();
        for(int i = 0; i < shape.dim_size(); i++) {
            resultShape.addDimension(shape.dim(i).size());
        }
    }
    return resultShape;
}

void TensorFlowEngine::run() {
	if(mInputNodes.empty())
		throw Exception("At least one output node has to be given to the NeuralNetwork before execution");
	if(mOutputNodes.empty())
		throw Exception("At least one output node has to be given to the NeuralNetwork before execution");

	// For each input, create a tensorflow tensor:
	std::vector <std::pair<std::string, tensorflow::Tensor>> input_tensors;
	for(auto inputNode : mInputNodes) {
		const std::string name = inputNode.first;
		if(!inputNode.second.data)
			throw Exception("Input node " + name + " has not received any data");
		auto shape = inputNode.second.data->getShape();
		if(shape.getUnknownDimensions() > 0)
		    throw Exception("Input shape must be fully known when executing NN");

		// Construct tensorflow tensor
        tensorflow::TensorShape tensorShape;
        for(auto i : shape.getAll()) {
            tensorShape.AddDim(i);
        }
        tensorflow::Tensor input_tensor(
                tensorflow::DT_FLOAT,
                tensorShape
        );

        // Give tensor data to tensorflow
        // TODO is the data here actually moved?
        TensorAccess::pointer access = inputNode.second.data->getAccess(ACCESS_READ);
        switch(shape.getDimensions()) {
            case 2:
                input_tensor.tensor<float, 2>() = std::move(access->getData<2>());
                break;
            case 3:
                input_tensor.tensor<float, 3>() = std::move(access->getData<3>());
                break;
            case 4:
                input_tensor.tensor<float, 4>() = std::move(access->getData<4>());
                break;
            case 5:
                input_tensor.tensor<float, 5>() = std::move(access->getData<5>());
                break;
			case 6:
				input_tensor.tensor<float, 6>() = std::move(access->getData<6>());
				break;
            default:
                throw Exception("Invalid tensor dimension size");
		}

		// Add tensorflow tensor to list of input tensors
		input_tensors.push_back(std::make_pair(name, input_tensor));
	}

    for(std::string name : mLearningPhaseTensors) {
        // Create a scalar tensor which tells the system we are NOT doing training
        tensorflow::Tensor input_tensor2(
                tensorflow::DT_BOOL,
                tensorflow::TensorShape() // Scalar
        );
        auto input_tensor_mapped2 = input_tensor2.tensor<bool, 0>();
        input_tensor_mapped2(0) = false;
        input_tensors.push_back(std::make_pair(name, input_tensor2));
    }

	std::vector<tensorflow::Tensor> output_tensors;

	reportInfo() << "Running network" << reportEnd();
	tensorflow::Status s;
	//mRuntimeManager->startRegularTimer("network_execution");
	std::vector<std::string> outputNames;
	for(auto node : mOutputNodes)
	    outputNames.push_back(node.first);
	s = mSession->Run(input_tensors, outputNames, {}, &output_tensors);
	//mRuntimeManager->stopRegularTimer("network_execution");

	if (!s.ok()) {
		throw Exception("Error during inference: " + s.ToString());
	}
	reportInfo() << "Finished executing network" << reportEnd();

    // Collect all output data as FAST tensors
    for(int j = 0; j < outputNames.size(); ++j) {
        const std::string outputName = outputNames[j];
        const NetworkNode node = mOutputNodes[outputName];
        auto tensor = TensorFlowTensor::create(new TensorFlowTensorWrapper(std::move(output_tensors[j])));
        mOutputNodes[outputName].data = tensor;
	}
	reportInfo() << "Finished parsing output" << reportEnd();
}


void TensorFlowEngine::load() {
    // Setup tensorflow session options
    tensorflow::SessionOptions options;
    tensorflow::ConfigProto &config = options.config;
    config.mutable_gpu_options()->set_allow_growth(true); // Set this so that tensorflow will not use up all GPU memory
    if (m_deviceType == InferenceDeviceType::CPU) {
        config.mutable_gpu_options()->set_visible_device_list(""); // Hide devices to force CPU execution
    } else if (m_deviceIndex >= 0) {
        config.mutable_gpu_options()->set_visible_device_list(std::to_string(m_deviceIndex)); // Use specific GPU
    }

	tensorflow::GraphDef tensorflow_graph;

    const auto networkFilename = getFilename();
	if(networkFilename.substr(networkFilename.size()-3) == ".pb" || tensorflow::MaybeSavedModelDirectory(networkFilename) == false) {
	    // Load a frozen protobuf file (.pb)
        if(!fileExists(networkFilename))
            throw Exception(networkFilename + " does not exist");
        reportInfo() << "Loading network file: " << networkFilename << reportEnd();
        tensorflow::Status s = ReadBinaryProto(tensorflow::Env::Default(), networkFilename, &tensorflow_graph);
        if (!s.ok()) {
            throw Exception("Could not read TensorFlow graph file " + networkFilename);
        }
        reportInfo() << "Creating session." << reportEnd();
        mSession.reset(tensorflow::NewSession(options));
        s = mSession->Create(tensorflow_graph);
        if (!s.ok()) {
            throw Exception("Could not create TensorFlow Graph: " + s.error_message());
        }
	} else {
	    // Load a model stored in the SavedModel format
        mSavedModelBundle = std::make_unique<tensorflow::SavedModelBundle>();
        tensorflow::RunOptions runOptions;
        auto s = tensorflow::LoadSavedModel(options, runOptions, networkFilename, {tensorflow::kSavedModelTagServe}, mSavedModelBundle.get());
        if(!s.ok()) {
            throw Exception("Could not read TensorFlow SavedModel from " + networkFilename);
        }
        tensorflow::MetaGraphDef metaGraphDef = mSavedModelBundle->meta_graph_def;
        mSession.reset(mSavedModelBundle->GetSession());
        tensorflow_graph = metaGraphDef.graph_def();
	}

	// Analyze nodes
    int inputCounter = 0;
	int outputCounter = 0;
    const bool inputsSpecified = !mInputNodes.empty();
	const bool outputsSpecified = !mOutputNodes.empty();

    for(int i = 0; i < tensorflow_graph.node_size(); ++i) {
		tensorflow::NodeDef node = tensorflow_graph.node(i);
        auto shape = getShape(node);
        // If node has not been specified by user, we need to add it
        // and thus know its type (fast image or tensor)
        // It is assumed to be an image if input shape has at least 4 dimensions
        NodeType type = NodeType::TENSOR;
        if(shape.getDimensions() >= 4) {
            //reportInfo() << "Assuming node is an image" << reportEnd();
            type = NodeType::IMAGE;
        } else if(shape.getDimensions() > 0) {
            //reportInfo() << "Assuming node is a tensor" << reportEnd();
        } else {
        }
        if(node.op() == "Placeholder") {
            if(shape.getDimensions() == 0)
                continue;
			if(node.name().find("keras_learning_phase") != std::string::npos) {
				//mLearningPhaseTensors.insert(node.name());
				mLearningPhaseTensors.push_back(node.name());
			} else {
				// Input node found:
				// Get its shape
				// Input nodes use the Op Placeholder
				reportInfo() << "Found input node: " << i << " with name " << node.name() << reportEnd();
				reportInfo() << "Node has shape " << shape.toString() << reportEnd();
				if(mInputNodes.count(node.name()) == 0) {
					reportInfo() << "Node was not specified by user" << reportEnd();

                    if(inputsSpecified) {
                        throw Exception("Encountered unknown node " + node.name());
                    }
					addInputNode(inputCounter, node.name(), type, shape);
					++inputCounter;
				} else {
				    // If shape that was given was empty, add the detected one here
				    if(mInputNodes[node.name()].shape.empty())
				        mInputNodes[node.name()].shape = shape;
				}
			}
        } else if(node.name().find("StatefulPartitionedCall") != std::string::npos) {
		    if(outputsSpecified) {
                reportInfo() << "Found output node " << node.name() << reportEnd();
                if(mOutputNodes.count(node.name()) > 0) {
		            reportInfo() << "Node was defined by user at id " << mOutputNodes[node.name()].portID  << reportEnd();
		            if(mOutputNodes[node.name()].shape.empty())
		                mOutputNodes[node.name()].shape = shape;
		        }
		    } else if(node.name() == "StatefulPartitionedCall") {
                reportWarning() << "No output nodes specified by user, FAST is guessing it is the node with name " << node.name() << reportEnd();
                addOutputNode(outputCounter, node.name(), type, shape);
                ++outputCounter;
		    }
		}
	}

    // If no output nodes, guess that last node is the output node
    if(mOutputNodes.empty()) {
        tensorflow::NodeDef node = tensorflow_graph.node(tensorflow_graph.node_size()-1);
        reportWarning() << "No output nodes were given to TensorFlow engine, FAST is guessing it is the last node with name " << node.name() << reportEnd();
        addOutputNode(0, node.name());
    }

	// Clear the proto to save memory space.
	tensorflow_graph.Clear();

	reportInfo() << "TensorFlow graph loaded from: " << networkFilename << reportEnd();

	setIsLoaded(true);
}

TensorFlowEngine::TensorFlowEngine() {
}

TensorFlowEngine::~TensorFlowEngine() {
	if(mSession) {
		mSession->Close();
	}
}

ImageOrdering TensorFlowEngine::getPreferredImageOrdering() const {
	return ImageOrdering::ChannelLast;
}

std::string TensorFlowEngine::getName() const {
    return "TensorFlow";
}

std::vector<InferenceDeviceInfo> TensorFlowEngine::getDeviceList() {
    std::vector<InferenceDeviceInfo> result;
    InferenceDeviceInfo cpu;
    cpu.type = InferenceDeviceType::CPU;
    cpu.index = 0;
    result.push_back(cpu);
#ifdef FAST_TENSORFLOW_CUDA
    {
        // TODO how to query number of devices?
        InferenceDeviceInfo device;
        device.type = InferenceDeviceType::GPU;
        device.index = 0;
        result.push_back(device);
    }
#endif
#ifdef FAST_TENSORFLOW_ROCM
    {
        // TODO ROCm how?
        InferenceDeviceInfo device;
        device.type = InferenceDeviceType::GPU;
        device.index = 0;
        result.push_back(device);
    }
#endif
    return result;
}

void TensorFlowEngine::loadCustomPlugins(std::vector<std::string> filenames) {
    if(isLoaded())
        throw Exception("You must call loadCustomPlugin before loading the model (e.g. load())");
    for(auto& filename : filenames) {
        if (!fileExists(filename))
            throw FileNotFoundException(filename);

        TF_Status *status = TF_NewStatus();
        TF_Library *library = TF_LoadLibrary(filename.c_str(), status);
        if(TF_GetCode(status) == TF_OK) {
            reportInfo() << "Plugin " << filename << " loaded in TensorFlowEngine" << reportEnd();
        } else {
            auto message = TF_Message(status);
            reportError() << "Plugin " << filename << " FAILED to load in TensorFlowEngine. Message: " << message << reportEnd();
        }
    }
}

}
