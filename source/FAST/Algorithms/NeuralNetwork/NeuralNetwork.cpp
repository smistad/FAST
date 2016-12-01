#include <caffe/include/caffe/blob.hpp>
#include "NeuralNetwork.hpp"
#include "FAST/Data/Image.hpp"
#include "FAST/Algorithms/ImageResizer/ImageResizer.hpp"

namespace fast {

// Get all available GPU devices
static void get_gpus(std::vector<int>* gpus) {
    int count = 0;
    count = caffe::Caffe::EnumerateDevices(true);
    for (int i = 0; i < count; ++i) {
      gpus->push_back(i);
    }
}

void NeuralNetwork::loadNetwork(std::string networkFilename) {
	std::vector<int> gpus;
	get_gpus(&gpus);
	if (gpus.size() != 0) {
		reportInfo() << "Use OpenCL device with ID " << gpus[0] << reportEnd();
		caffe::Caffe::SetDevices(gpus);
		caffe::Caffe::set_mode(caffe::Caffe::GPU);
		caffe::Caffe::SetDevice(gpus[0]);
	}
	FLAGS_minloglevel = 5; // Disable cout from caffe
	//caffe::Caffe::set_mode(caffe::Caffe::CPU);
	reportInfo() << "Loading neural network.." << reportEnd();
	mNetwork = SharedPointer<caffe::Net<float> >(new caffe::Net<float>(networkFilename, caffe::TEST, caffe::Caffe::GetDefaultDevice()));
	reportInfo() << "Finished loading neural network" << reportEnd();
	mModelLoaded = false;
}

void NeuralNetwork::loadWeights(std::string weightsFilename) {
    // Network must be loaded before weights
	if(!mNetwork.isValid()) {
		throw Exception("Must load network definition before loading weights");
	}

	reportInfo() << "Loading training file.." << reportEnd();
	mNetwork->CopyTrainedLayersFrom(weightsFilename);
	reportInfo() << "Finished loading training file." << reportEnd();
    mModelLoaded = true;
}

void NeuralNetwork::loadNetworkAndWeights(std::string networkFilename, std::string weightsFilename) {
    loadNetwork(networkFilename);
    loadWeights(weightsFilename);
}

NeuralNetwork::NeuralNetwork() {
	createInputPort<Image>(0, true, INPUT_STATIC_OR_DYNAMIC, true);
	mModelLoaded = false;
	mMeanImageLoaded = false;
	createOpenCLProgram(std::string(FAST_SOURCE_DIR) + "Algorithms/NeuralNetwork/NeuralNetwork.cl");
}

void NeuralNetwork::execute() {
    if(!mModelLoaded)
		throw Exception("Network and weights must be loaded in NeuralNetwork before execution.");

	std::vector<Image::pointer> images = getMultipleStaticInputData<Image>();

	// Assuming only one input layer here
	caffe::Blob<float>* input_layer = mNetwork->input_blobs()[0];

    if(input_layer->num() != images.size()) {
		// Only reshape if necessary
		// batch size x channels x width x height
		input_layer->Reshape(images.size(), input_layer->channels(), input_layer->height(), input_layer->width());
		mNetwork->Reshape();
		reportInfo() << "Net reshaped" << reportEnd();
	}

	// Pre processing, subtract mean, resize, crop, convert to normalized float etc.

    // TODO Resize images if necessary
    images = resizeImages(images);

	// Subtract mean
    if(mMeanImageLoaded) {
		// Mean image has been loaded, subtract it from each image
		images = subtractMeanImage(images);
	}

	// TODO normalization?

	executeNetwork(images);
}

std::vector<float> NeuralNetwork::getNetworkOutput() {
	caffe::Blob<float>* layerData = mNetwork->output_blobs()[0];
	std::vector<float> result(
			layerData->cpu_data(),
            layerData->cpu_data() + layerData->num()*layerData->channels()*layerData->height()*layerData->width()
	);
    return result;
}

std::vector<float> NeuralNetwork::getNetworkOutput(std::string layerName) {
    boost::shared_ptr<caffe::Blob<float> > layerData = mNetwork->blob_by_name(layerName);
    std::vector<float> result(
			layerData->cpu_data(),
			layerData->cpu_data() + layerData->num()*layerData->channels()*layerData->height()*layerData->width()
	);
	return result;
}

void NeuralNetwork::executeNetwork(const std::vector<Image::pointer>& images) {
 	// TODO, gives images directly to GPU
	// Set images to input layer
	reportInfo() << "Adding input data to input layer.." << reportEnd();
	int counter = 0;
	caffe::Blob<float>* input_layer = mNetwork->input_blobs()[0];
	float* input_data = input_layer->mutable_cpu_data(); // This is the input data layer
	for(Image::pointer image : images) {
        // Some sanity checks
		Vector3ui size = image->getSize();
		if(size.x() != input_layer->width() || size.y() != input_layer->height())
			throw Exception("Mismatch between size of input layer and an image given to NeuralNetwork::executeNetwork()");
		if(image->getDataType() != TYPE_FLOAT)
			throw Exception("Data type of images feed to executeNetwork() must be float");
		ImageAccess::pointer access = image->getImageAccess(ACCESS_READ);
		float* pixels = (float*)access->get();
		for(int i = 0; i < size.x()*size.y(); ++i) {
			input_data[counter + i] = pixels[i];
		}
		counter += size.x()*size.y();
	}

	// Do a forward pass
	reportInfo() << "Neural network foward pass executing..." << reportEnd();
	mNetwork->Forward();
	reportInfo() << "Neural network foward pass finished" << reportEnd();
}

std::vector<SharedPointer<Image>> NeuralNetwork::resizeImages(const std::vector<SharedPointer<Image>> &images) {
	reportInfo() << "Resizing images.." << reportEnd();
	caffe::Blob<float>* input_layer = mNetwork->input_blobs()[0];
    std::vector<Image::pointer> resizedImages;
	for(Image::pointer image : images) {
		// Resize image to fit input layer
		if(input_layer->width() != image->getWidth() || input_layer->height() != image->getHeight()) {
			// Only resize if needed
            ImageResizer::pointer resizer = ImageResizer::New();
            resizer->setWidth(input_layer->width());
            resizer->setHeight(input_layer->height());
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

void NeuralNetwork::loadBinaryMeanImage(std::string filename) {
	// Network must be loaded first
	if(!mNetwork.isValid()) {
		throw Exception("Must load network definition before loading weights");
	}

	reportInfo() << "Loading mean image file.." << reportEnd();
	caffe::BlobProto blob_proto;
	caffe::ReadProtoFromBinaryFileOrDie(filename.c_str(), &blob_proto);

	caffe::Blob<float> mMeanBlob;
	mMeanBlob.FromProto(blob_proto);
	OpenCLDevice::pointer device = getMainDevice();
	caffe::Blob<float>* input_layer = mNetwork->input_blobs()[0];
    // Assuming float image here
	mMeanImage = cl::Image2D(
			device->getContext(),
			CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			getOpenCLImageFormat(device, CL_MEM_OBJECT_IMAGE2D, TYPE_FLOAT, input_layer->channels()),
			input_layer->width(), input_layer->height(),
			0,
			mMeanBlob.mutable_cpu_data()
	);
	reportInfo() << "Finished loading mean image file." << reportEnd();

	mMeanImageLoaded = true;
}

std::vector<SharedPointer<Image> > NeuralNetwork::subtractMeanImage(const std::vector<SharedPointer<Image> >& images) {
	if(!mMeanImageLoaded) {
		throw Exception("Mean image not loaded, cannot subtract mean image from images");
	}

	reportInfo() << "Subtracting mean image.." << reportEnd();
	OpenCLDevice::pointer device = getMainDevice();
	cl::Program program = getOpenCLProgram(device);
	cl::Kernel normalizationKernel(program, "subtractMeanImage");
	normalizationKernel.setArg(1, mMeanImage);

	std::vector<Image::pointer> preProcessedImages;
	for(auto &&image : images) {
        Image::pointer preProcessedImage = Image::New();
		preProcessedImage->create(image->getSize(), TYPE_FLOAT, 1);
		OpenCLImageAccess::pointer access = image->getOpenCLImageAccess(ACCESS_READ, device);
		OpenCLImageAccess::pointer access2 = preProcessedImage->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
		normalizationKernel.setArg(0, *(access->get2DImage()));
		normalizationKernel.setArg(2, *(access2->get2DImage()));

		device->getCommandQueue().enqueueNDRangeKernel(
				normalizationKernel,
				cl::NullRange,
				cl::NDRange(image->getWidth(), image->getHeight()),
				cl::NullRange
		);

		preProcessedImages.push_back(preProcessedImage);

	}

	return preProcessedImages;
}


};
