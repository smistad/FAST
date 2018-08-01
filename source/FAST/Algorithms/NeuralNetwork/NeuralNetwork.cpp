#include "NeuralNetwork.hpp"
#include "FAST/Data/Image.hpp"
#include "FAST/Data/Tensor.hpp"
#include "FAST/Algorithms/ImageResizer/ImageResizer.hpp"
#include <tensorflow/core/framework/step_stats.pb.h>
#include <tensorflow/core/framework/tensor.h>
#include <tensorflow/core/framework/types.pb.h>
#include <tensorflow/core/lib/strings/stringprintf.h>
#include <tensorflow/core/platform/env.h>
#include <tensorflow/core/platform/logging.h>
#include <tensorflow/core/platform/mutex.h>
#include <tensorflow/core/platform/types.h>
#include <tensorflow/core/public/session.h>
#include <tensorflow/core/graph/default_device.h>
#include <tensorflow/core/platform/init_main.h>
#include <tensorflow/cc/framework/ops.h>

namespace fast {

/**
 * This specialized Tensor Data class, allow us to store Tensorflow type tensors as FAST tensors.
 */
class TensorflowTensor : public Tensor {
    FAST_OBJECT(TensorflowTensor)
    public:
        void create(tensorflow::Tensor&& tensorflowTensor);
        TensorAccess::pointer getAccess(accessType access) override;
    private:
        tensorflow::Tensor m_tensorflowTensor;
};

void TensorflowTensor::create(tensorflow::Tensor&& tensorflowTensor) {
    m_tensorflowTensor = tensorflowTensor;
    auto shape = m_tensorflowTensor.shape();
    for(int i = 0; i < shape.dims(); ++i)
        m_shape.addDimension(shape.dim_size(i));
}

TensorAccess::pointer TensorflowTensor::getAccess(accessType type) {
    // TODO process type
    return std::make_unique<TensorAccess>(m_tensorflowTensor.flat<float>().data(), m_shape, std::static_pointer_cast<Tensor>(mPtr.lock()));
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

void NeuralNetwork::load(std::string networkFilename) {

	tensorflow::SessionOptions options;
	tensorflow::ConfigProto &config = options.config;
	tensorflow::GPUOptions* gpuOptions = config.mutable_gpu_options();
	gpuOptions->set_allow_growth(true); // Set this so that tensorflow will not use up all GPU memory
	//gpuOptions->set_per_process_gpu_memory_fraction(0.5);
	mSession.reset(tensorflow::NewSession(options));
	tensorflow::GraphDef tensorflow_graph;

	{
		tensorflow::Status s = ReadBinaryProto(tensorflow::Env::Default(), networkFilename, &tensorflow_graph);
		if (!s.ok()) {
			throw Exception("Could not read TensorFlow graph file " + networkFilename);
		}
	}

	bool nodesSpecified = true;
    int inputCounter = 0;
	if(mInputNodes.size() == 0) {
		nodesSpecified = false;
	}
    for(int i = 0; i < tensorflow_graph.node_size(); ++i) {
		tensorflow::NodeDef node = tensorflow_graph.node(i);
		if(mInputNodes.count(node.name()) > 0) {
		}
		if(node.op() == "Placeholder") {
			if(node.name().find("keras_learning_phase") != std::string::npos) {
				mLearningPhaseTensors.push_back(node.name());
			} else {
				// Input node found:
				// Get its shape
				// Input nodes use the Op Placeholder
				reportInfo() << "Found input node: " << i << " with name " << node.name() << reportEnd();
				auto shape = getShape(node);
				if(mInputNodes.count(node.name()) == 0) {
					if(nodesSpecified) {
						throw Exception("Encountered unknown node " + node.name());
					}
					reportInfo() << "Node was not specified by user" << reportEnd();
					// If node has not been specified by user, we need to add it
					// and thus know its type (fast image or tensor)
					// It is assumed to be an image if input shape has at least 4 dimensions
					NodeType type = NodeType::TENSOR;
					if(shape.getKnownDimensions() >= 2) {
						reportInfo() << "Assuming node is an image" << reportEnd();
						type = NodeType::IMAGE;
					} else {
						reportInfo() << "Assuming node is a tensor" << reportEnd();
					}
					addInputNode(inputCounter, tensorflow_graph.node(0).name(), type, shape);
					++inputCounter;
				}

				// Set its shape
				mInputNodes[node.name()].shape = shape;
			}
		}
	}

	reportInfo() << "Creating session." << reportEnd();
	tensorflow::Status s = mSession->Create(tensorflow_graph);
	if (!s.ok()) {
		throw Exception("Could not create TensorFlow Graph");
	}

	//tensorflow::graph::SetDefaultDevice("/gpu:0", &tensorflow_graph);

	// Clear the proto to save memory space.
	tensorflow_graph.Clear();
	reportInfo() << "TensorFlow graph loaded from: " << networkFilename << reportEnd();

	mModelLoaded = true;
}

void NeuralNetwork::setScaleFactor(float factor) {
    mScaleFactor = factor;
}

void NeuralNetwork::setPreserveAspectRatio(bool preserve) {
    mPreserveAspectRatio = preserve;
}

void NeuralNetwork::setHorizontalFlipping(bool flip) {
	mHorizontalImageFlipping = flip;
}

NeuralNetwork::NeuralNetwork() {
	mModelLoaded = false;
	mPreserveAspectRatio = false;
	mScaleFactor = 1.0f;
	createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/NeuralNetwork/NeuralNetwork.cl");
	createStringAttribute("model", "Model path", "Path to neural network tensorflow model", "");
	createIntegerAttribute("input_size", "Input size", "Image input size", 128);
	createFloatAttribute("scale_factor", "Scale factor", "Scale factor", mScaleFactor);
	createStringAttribute("output_names", "Output names", "Name of output nodes", "");
	createBooleanAttribute("signed_input_normalization", "Signed input normalization", "Normalize input to -1 and 1 instead of 0 to 1.", false);
}

std::unordered_map<std::string, std::vector<Tensor::pointer>> NeuralNetwork::processInputData() {
    std::unordered_map<std::string, std::vector<Tensor::pointer>> tensors;
    for(auto inputNode : mInputNodes) {
        // TODO batch detection
        auto shape = inputNode.second.shape;
        if(shape.getDimensions() == 0)
            throw Exception("Unable to deduce input shape from .pb file. Either export the file with shape information or supply the input shape manually using setInputShape");

        SharedPointer<DataObject> data = getInputData<DataObject>(inputNode.second.portID);

        // Check if data object is an image by doing a dynamic cast
        Image::pointer image = std::dynamic_pointer_cast<Image>(data);
        if(image) {
            mImages.push_back(image);
            while(mImages.size() < mTemporalWindow)
                mImages.push_back(image);
            while(mImages.size() > mTemporalWindow)
                mImages.pop_front();

            std::vector<Image::pointer> inputImages(mImages.begin(), mImages.end());

            if(shape.getDimensions() < 4)
            	throw Exception("Trying to attach an image to an input node with shape with fewer than 4 dimensions");

            // Resize images to fit input
            inputImages = resizeImages(inputImages, shape[2], shape[1]);

			std::vector<Tensor::pointer> inputTensors;
            // Convert images to tensors
            for(auto image : inputImages) {
            	inputTensors.push_back(convertImageToTensor(image, inputNode.second.shape));
            }
			tensors[inputNode.first] = inputTensors;
        } else {
            Tensor::pointer tensor = std::dynamic_pointer_cast<Tensor>(data);
            mTensors.push_back(tensor);
            while(mTensors.size() < mTemporalWindow)
                mTensors.push_back(tensor);
            while(mTensors.size() > mTemporalWindow)
                mTensors.pop_front();
            std::vector<Tensor::pointer> inputTensors(mTensors.begin(), mTensors.end());
            tensors[inputNode.first] = inputTensors;
        }
	}

	return tensors;
}

void NeuralNetwork::execute() {
	// Add output tensors to the output of this PO
	auto inputTensors = processInputData();
	auto output = executeNetwork(inputTensors);
    for(auto outputTensor : output) {
        auto node = outputTensor.first;
        auto tensor = outputTensor.second;
        addOutputData(node.portID, tensor);
    }
}

Tensor::pointer NeuralNetwork::convertImageToTensor(Image::pointer image, const TensorShape& shape) {
    // Create input tensor

    OpenCLDevice::pointer device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
    cl::Program program = getOpenCLProgram(device);
    cl::Kernel kernel(program, "normalizeInput");
	const int height = shape[1];
    const int width = shape[2];
    OpenCLImageAccess::pointer access = image->getOpenCLImageAccess(ACCESS_READ, device);
    cl::Buffer buffer(
            device->getContext(),
            CL_MEM_WRITE_ONLY,
            sizeof(float) * width * height
    );
    kernel.setArg(0, *access->get2DImage());
    kernel.setArg(1, buffer);
    kernel.setArg(2, mScaleFactor);
    kernel.setArg(3, (int) (mHorizontalImageFlipping ? 1 : 0));
    kernel.setArg(4, (int) (mSignedInputNormalization ? 1 : 0));

    device->getCommandQueue().enqueueNDRangeKernel(
            kernel,
            cl::NullRange,
            cl::NDRange(width, height),
            cl::NullRange
    );

    auto values = make_uninitialized_unique<float[]>(width * height);
    device->getCommandQueue().enqueueReadBuffer(buffer, CL_TRUE, 0, sizeof(float) * width * height,
                                                values.get());

    if(image->getWidth() != width || image->getHeight() != height)
        throw Exception("Input image sent to executeNetwork was of incrorrect size");

    auto tensor = Tensor::New();
    tensor->create(std::move(values), {1, height, width, 1});
    return tensor;
}

std::vector<std::pair<NeuralNetwork::NetworkNode, Tensor::pointer>> NeuralNetwork::executeNetwork(std::unordered_map<std::string, std::vector<Tensor::pointer>> tensors) {
    if(!mModelLoaded)
		throw Exception("Network and weights must be loaded in NeuralNetwork before execution.");
	if(mOutputNodes.size() == 0)
		throw Exception("At least one output node has to be given to the NeuralNetwork before execution");

    const int batchSize = 1;

	// For each input, create a tensorflow tensor:
	std::vector <std::pair<std::string, tensorflow::Tensor>> input_tensors;
	for(auto inputNode : mInputNodes) {
		const std::string name = inputNode.first;
		auto shape = inputNode.second.shape;
		shape[0] = batchSize;

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
        TensorAccess::pointer access = tensors[name][0]->getAccess(ACCESS_READ);
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
	mRuntimeManager->startRegularTimer("network_execution");
	std::vector<std::string> outputNames;
	for(auto node : mOutputNodes)
	    outputNames.push_back(node.first);
	s = mSession->Run(input_tensors, outputNames, {}, &output_tensors);
	mRuntimeManager->stopRegularTimer("network_execution");

	if (!s.ok()) {
		throw Exception("Error during inference: " + s.ToString());
	}
	reportInfo() << "Finished executing network" << reportEnd();

    // Collect all output data as FAST tensors
    std::vector<std::pair<NetworkNode, Tensor::pointer>> result;
    for(int j = 0; j < outputNames.size(); ++j) {
        const std::string outputName = outputNames[j];
        const NetworkNode node = mOutputNodes[outputName];

        auto tensor = TensorflowTensor::New();
        tensor->create(std::move(output_tensors[j]));
        result.push_back(std::make_pair(node, tensor));
	}
	reportInfo() << "Finished parsing output" << reportEnd();

    return result;
}

std::vector<SharedPointer<Image>> NeuralNetwork::resizeImages(const std::vector<SharedPointer<Image>> &images, int width, int height) {
    mRuntimeManager->startRegularTimer("image input resize");
    std::vector<Image::pointer> resizedImages;
	for(Image::pointer image : images) {
		// Resize image to fit input layer
		if(width != image->getWidth() || height != image->getHeight()) {
			// Only resize if needed
            ImageResizer::pointer resizer = ImageResizer::New();
            resizer->setWidth(width);
            resizer->setHeight(height);
            resizer->setInputData(image);
			resizer->setPreserveAspectRatio(mPreserveAspectRatio);
			DataPort::pointer port = resizer->getOutputPort();
            resizer->update(0);
            Image::pointer resizedImage = port->getNextFrame<Image>();
            mNewInputSpacing = resizedImage->getSpacing();
            resizedImages.push_back(resizedImage);
		} else {
			mNewInputSpacing = image->getSpacing();
			resizedImages.push_back(image);
		}
	}
	mRuntimeManager->stopRegularTimer("image input resize");

	return resizedImages;
}

void NeuralNetwork::loadAttributes() {
	load(getStringAttribute("model"));
	std::vector<int> inputSize = getIntegerListAttribute("input_size");
	// TODO Fix
	//setInputSize(inputSize.at(0), inputSize.at(1));
	std::vector<std::string> outputNames = getStringListAttribute("output_names");
	//setOutputParameters(outputNames);
	setScaleFactor(getFloatAttribute("scale_factor"));
	setSignedInputNormalization(getBooleanAttribute("signed_input_normalization"));
}

void NeuralNetwork::setTemporalWindow(uint window) {
	if(window < 1) {
        throw Exception("Remember frames has to be > 0.");
	}
	mTemporalWindow = window;
}

void NeuralNetwork::setSignedInputNormalization(bool signedInputNormalization) {
	mSignedInputNormalization = signedInputNormalization;
}

void NeuralNetwork::addTemporalImageFrame(SharedPointer<Image> image) {
	mImages.push_back(image);
}

NeuralNetwork::~NeuralNetwork() {
	if(mSession.get() != nullptr) {
		mSession->Close();
	}
}

void NeuralNetwork::addInputNode(uint portID, std::string name, NeuralNetwork::NodeType type, TensorShape shape) {
	NetworkNode node;
	node.portID = portID;
	node.type = type;
	node.shape = shape;
	mInputNodes[name] = node;
	createInputPort<DataObject>(portID);
}

void NeuralNetwork::addOutputNode(uint portID, std::string name, NeuralNetwork::NodeType type, TensorShape shape) {
	NetworkNode node;
	node.portID = portID;
	node.type = type;
	node.shape = shape;
	mOutputNodes[name] = node;
	createOutputPort<DataObject>(portID);
}

};
