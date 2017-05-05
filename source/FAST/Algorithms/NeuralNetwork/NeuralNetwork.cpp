#include "NeuralNetwork.hpp"
#include "FAST/Data/Image.hpp"
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

namespace fast {

// See here for reference: https://github.com/tensorflow/tensorflow/blob/86f5ab7474825da756838b34e1b4eac93f5fc68a/tensorflow/contrib/android/jni/tensorflow_inference_jni.cc

void NeuralNetwork::load(std::string networkFilename) {

	char** argv = new char*[1];
	argv[0] = new char[255];
    int argc = 1;
    tensorflow::port::InitMain(argv[0], &argc, &argv);
	tensorflow::SessionOptions options;
	tensorflow::ConfigProto &config = options.config;
	mSession.reset(tensorflow::NewSession(options));
	tensorflow::GraphDef tensorflow_graph;

	{
		tensorflow::Status s = ReadBinaryProto(tensorflow::Env::Default(), networkFilename, &tensorflow_graph);
		if (!s.ok()) {
			throw Exception("Could not read TensorFlow graph file " + networkFilename);
		}
	}

	// Assume first node is input node
	mInputName = tensorflow_graph.node(0).name();
    //auto attributes = tensorflow_graph.node(0).attr();
	//std::cout << attributes["shape"].shape() << std::endl;
    for(int i = 0; i < tensorflow_graph.node_size(); ++i) {
		tensorflow::NodeDef node = tensorflow_graph.node(i);
        //reportInfo() << "Node " << i << " with name " << node.name() << reportEnd();
        //reportInfo() << "Op name " << node.op() << reportEnd();
        //reportInfo() << "inputs: " << node.input_size() << reportEnd();
        if(node.name() == "keras_learning_phase") {
			mHasKerasLearningPhaseTensor = true;
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

NeuralNetwork::NeuralNetwork() {
	createInputPort<Image>(0, true, INPUT_STATIC_OR_DYNAMIC, true);
	mModelLoaded = false;
	mHasKerasLearningPhaseTensor = false;
	mInputName = "";
	mWidth = -1;
	mHeight = -1;
	mScaleFactor = 1.0f;
	createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/NeuralNetwork/NeuralNetwork.cl");
	createStringAttribute("model", "Model path", "Path to neural network tensorflow model", "");
	createIntegerAttribute("input_size", "Input size", "Image input size", 128);
	createFloatAttribute("scale_factor", "Scale factor", "Scale factor", mScaleFactor);
	createStringAttribute("output_names", "Output names", "Name of output nodes", "");
}

void NeuralNetwork::execute() {


    mImage = getStaticInputData<Image>();
	std::vector<Image::pointer> images = {mImage};//getMultipleStaticInputData<Image>();

	if(mWidth < 0 || mHeight < 0)
		throw Exception("Network input layer width and height has to be specified before running the network");

    images = resizeImages(images);

	executeNetwork(images);
}


void NeuralNetwork::setInputSize(int width, int height) {
	mWidth = width;
	mHeight = height;
}
void NeuralNetwork::setOutputParameters(std::vector<std::string> outputNodeNames) {
    mOutputNames = outputNodeNames;
}

tensorflow::Tensor NeuralNetwork::getNetworkOutput() {
    if(mOutputNames.size() != 1)
		throw Exception("If network has more than 1 output can't return network output without name.");

	return mOutputData[mOutputNames[0]];
}

tensorflow::Tensor NeuralNetwork::getNetworkOutput(std::string name) {
	return mOutputData.at(name);
}

void NeuralNetwork::executeNetwork(const std::vector<Image::pointer>& images) {
    if(!mModelLoaded)
		throw Exception("Network and weights must be loaded in NeuralNetwork before execution.");
	if(mInputName == "")
		throw Exception("An input name must ge given to the NeuralNetwork before execution");
	if(mOutputNames.size() == 0)
		throw Exception("An output name must ge given to the NeuralNetwork before execution");

    int batchSize = images.size();
	if(batchSize == 0)
		throw Exception("Need at least one image to execute network.");

	// Create input tensor
	tensorflow::Tensor input_tensor(
			tensorflow::DT_FLOAT,
			tensorflow::TensorShape({batchSize, mHeight, mWidth, 1})
	);

	auto input_tensor_mapped = input_tensor.tensor<float, 4>();

	mRuntimeManager->startRegularTimer("input_data_copy");
	reportInfo() << "TensorFlow: Copying Data." << reportEnd();
	for(int n = 0; n < batchSize; ++n) {
		Image::pointer image = images[n];
		if (image->getWidth() != mWidth || image->getHeight() != mHeight)
			throw Exception("Input image sent to executeNetwork was of incrorrect size");

		ImageAccess::pointer access = image->getImageAccess(ACCESS_READ);
		for (int i = 0; i < mHeight; ++i) { // y
			for (int j = 0; j < mWidth; ++j) { // x
				input_tensor_mapped(n, i, j, 0) = access->getScalar(Vector2i(j, i))*mScaleFactor;
			}
		}
	}
	mRuntimeManager->stopRegularTimer("input_data_copy");

    // TODO Need to know names of inputs and outputs in advance
	// Input: Only single for now
	// Output: Can be multiple


	std::vector <std::pair<std::string, tensorflow::Tensor>> input_tensors(
			{{mInputName, input_tensor}});

	if(mHasKerasLearningPhaseTensor) {
		// Create a scalar tensor which tells the system we are NOT doing training
		tensorflow::Tensor input_tensor2(
				tensorflow::DT_BOOL,
				tensorflow::TensorShape() // Scalar
		);
		auto input_tensor_mapped2 = input_tensor2.tensor<bool, 0>();
		input_tensor_mapped2(0) = false;
		input_tensors.push_back(std::make_pair("keras_learning_phase", input_tensor2));
	}

	std::vector <tensorflow::Tensor> output_tensors;

	reportInfo() << "Running network" << reportEnd();
	tensorflow::Status s;
	mRuntimeManager->startRegularTimer("network_execution");
	s = mSession->Run(input_tensors, mOutputNames, {}, &output_tensors);
	mRuntimeManager->stopRegularTimer("network_execution");

	if (!s.ok()) {
		throw Exception("Error during inference: " + s.ToString());
	}
	reportInfo() << "Finished executing network" << reportEnd();
    // Store all output data
    for(int j = 0; j < mOutputNames.size(); ++j) {
        std::string outputName = mOutputNames[j];
		mOutputData[outputName] = output_tensors[j];
	}
	reportInfo() << "Finished parsing output" << reportEnd();

}

std::vector<SharedPointer<Image>> NeuralNetwork::resizeImages(const std::vector<SharedPointer<Image>> &images) {
	reportInfo() << "Resizing images.." << reportEnd();
    std::vector<Image::pointer> resizedImages;
	for(Image::pointer image : images) {
		// Resize image to fit input layer
		if(mWidth != image->getWidth() || mHeight != image->getHeight()) {
			// Only resize if needed
            ImageResizer::pointer resizer = ImageResizer::New();
            resizer->setWidth(mWidth);
            resizer->setHeight(mHeight);
            resizer->setInputData(image);
            resizer->update();
            Image::pointer resizedImage = resizer->getOutputData<Image>();
            resizedImages.push_back(resizedImage);
		} else {
			resizedImages.push_back(image);
		}
	}

	return resizedImages;
}

void NeuralNetwork::loadAttributes() {
	load(getStringAttribute("model"));
	std::vector<int> inputSize = getIntegerListAttribute("input_size");
	setInputSize(inputSize.at(0), inputSize.at(1));
	std::vector<std::string> outputNames = getStringListAttribute("output_names");
	setOutputParameters(outputNames);
	setScaleFactor(getFloatAttribute("scale_factor"));
}

};
