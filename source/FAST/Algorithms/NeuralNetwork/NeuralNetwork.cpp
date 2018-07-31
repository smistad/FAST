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

// See here for reference: https://github.com/tensorflow/tensorflow/blob/86f5ab7474825da756838b34e1b4eac93f5fc68a/tensorflow/contrib/android/jni/tensorflow_inference_jni.cc

static std::vector<int> getShape(const tensorflow::NodeDef& node) {
    std::vector<int> resultShape;
    if(node.attr().count("shape") > 0) {
        auto shape = node.attr().at("shape").shape();
        // Drop batch size
        for(int i = 1; i < shape.dim_size(); i++) {
            resultShape.push_back(shape.dim(i).size());
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

	// Assume first node is input node
	if(mInputNodes.size() == 0) {
		addInputNode(0, tensorflow_graph.node(0).name(), NodeType::IMAGE);
	}

    for(int i = 0; i < tensorflow_graph.node_size(); ++i) {
		tensorflow::NodeDef node = tensorflow_graph.node(i);
		if(mInputNodes.count(node.name()) > 0) {
			// Input node found, set its shape
		    mInputNodes[node.name()].shape = getShape(node);
		}
		/*
        reportInfo() << "Node " << i << " with name " << node.name() << reportEnd();
        reportInfo() << "Op name " << node.op() << reportEnd();
        reportInfo() << "inputs: " << node.input_size() << reportEnd();
        */

        if(node.name().find("keras_learning_phase") != std::string::npos) {
			mLearningPhaseTensors.push_back(node.name());
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

std::pair<std::unordered_map<std::string, std::vector<Image::pointer>>, std::unordered_map<std::string, std::vector<Tensor::pointer>>> NeuralNetwork::processInputData() {
    std::unordered_map<std::string, std::vector<Image::pointer>> images;
    std::unordered_map<std::string, std::vector<Tensor::pointer>> tensors;
    for(auto inputNode : mInputNodes) {
        // TODO batch detection
        auto shape = inputNode.second.shape;
        if(shape.size() == 0)
            throw Exception("Unable to deduce input shape from .pb file. Either export the file with shape information or supply the input shape manually using setInputShape");

        if(inputNode.second.type == NodeType::IMAGE) {
            // Input node wants images
            Image::pointer image = getInputData<Image>(inputNode.second.portID);

            mImages.push_back(image);
            while(mImages.size() < mTemporalWindow)
                mImages.push_back(image);
            while(mImages.size() > mTemporalWindow)
                mImages.pop_front();

            std::vector<Image::pointer> inputImages(mImages.begin(), mImages.end());
            images[inputNode.first] = resizeImages(inputImages, shape[1], shape[0]);
        } else {
            // Input node wants tensors
            Tensor::pointer tensor = getInputData<Tensor>(inputNode.second.portID);
            mTensors.push_back(tensor);
            while(mTensors.size() < mTemporalWindow)
                mTensors.push_back(tensor);
            while(mTensors.size() > mTemporalWindow)
                mTensors.pop_front();
            std::vector<Tensor::pointer> inputTensors(mTensors.begin(), mTensors.end());
            tensors[inputNode.first] = inputTensors;
        }
	}

	return std::make_pair(images, tensors);
}

void NeuralNetwork::execute() {
	// Add output tensors to the output of this PO
	auto data = processInputData();
	auto output = executeNetwork(data.first, data.second);
    for(auto outputTensor : output) {
        auto node = outputTensor.first;
        auto tensor = outputTensor.second;
        addOutputData(node.portID, tensor);
    }
}

TensorData<4> NeuralNetwork::convertImageToTensor(Image::pointer image, const NetworkNode& node) {
    const auto shape = node.shape;
    // Create input tensor

    OpenCLDevice::pointer device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
    cl::Program program = getOpenCLProgram(device);
    cl::Kernel kernel(program, "normalizeInput");
    const int width = shape[0];
    const int height = shape[1];
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

    return TensorData<4>(values.get(), 1, height, width, 1);
}

std::vector<std::pair<NeuralNetwork::NetworkNode, Tensor::pointer>> NeuralNetwork::executeNetwork(std::unordered_map<std::string, std::vector<SharedPointer<Image>>>& images, std::unordered_map<std::string, std::vector<SharedPointer<Tensor>>>& tensors) {
    if(!mModelLoaded)
		throw Exception("Network and weights must be loaded in NeuralNetwork before execution.");
	if(mOutputNodes.size() == 0)
		throw Exception("At least one output node has to be given to the NeuralNetwork before execution");

    const int batchSize = 1;//images.size();
	if(batchSize == 0)
		throw Exception("Need at least one image to execute network.");

    mRuntimeManager->startRegularTimer("input_data_copy");
	// For each input, create a tensorflow tensor:
	std::vector <std::pair<std::string, tensorflow::Tensor>> input_tensors;
	for(auto inputNode : mInputNodes) {
		const std::string name = inputNode.first;
		const auto shape = inputNode.second.shape;

		// Construct tensorflow tensor
        tensorflow::TensorShape tensorShape;
        tensorShape.AddDim(batchSize);
        for(auto i : shape) {
            tensorShape.AddDim(i);
        }
        tensorflow::Tensor input_tensor(
                tensorflow::DT_FLOAT,
                tensorShape
        );

        // Assign data to the tensor
		if(inputNode.second.type == NodeType::IMAGE) {
		    // Convert image to tensor
		    input_tensor.tensor<float, 4>() = std::move(convertImageToTensor(images[name][0], inputNode.second));
		} else {
		    // Give tensor data to tensorflow
		    TensorAccess::pointer access = tensors[name][0]->getAccess(ACCESS_READ);
		    switch(shape.size()) {
		        case 1:
		            input_tensor.tensor<float, 1>() = std::move(access->getData<1>());
		            break;
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
		        default:
		            throw Exception("Invalid tensor dimension size");
		    }
		}

		// Add tensorflow tensor to list of input tensors
		input_tensors.push_back(std::make_pair(name, input_tensor));
	}
	mRuntimeManager->stopRegularTimer("input_data_copy");


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
    // Store all output data
    std::vector<std::pair<NetworkNode, Tensor::pointer>> result;
    for(int j = 0; j < outputNames.size(); ++j) {
        const std::string outputName = outputNames[j];
        const NetworkNode node = mOutputNodes[outputName];

        auto tensor = Tensor::New();
        auto tensorflowTensor = output_tensors[j];
        auto tensorflowShape = tensorflowTensor.shape();
        std::vector<int> shape;
        for(int i = 0; i < tensorflowShape.dims(); ++i) {
            shape.push_back(tensorflowShape.dim_size(i));
        }
        // Copy data from tensorflow to FAST
        // TODO is there anyway to avoid this copy???
        auto tensorflowData = tensorflowTensor.flat<float>();
        auto copiedData = std::make_unique<float[]>(tensorflowData.size());
        std::memcpy(copiedData.get(), tensorflowData.data(), sizeof(float)*tensorflowData.size());
        tensor->create(std::move(copiedData), shape);
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

void NeuralNetwork::addInputNode(uint portID, std::string name, NeuralNetwork::NodeType type, std::vector<int> shape) {
	NetworkNode node;
	node.portID = portID;
	node.type = type;
	node.shape = shape;
	mInputNodes[name] = node;
	createInputPort<DataObject>(portID);
}

void NeuralNetwork::addOutputNode(uint portID, std::string name, NeuralNetwork::NodeType type, std::vector<int> shape) {
	NetworkNode node;
	node.portID = portID;
	node.type = type;
	node.shape = shape;
	mOutputNodes[name] = node;
	createOutputPort<DataObject>(portID);
}

};
