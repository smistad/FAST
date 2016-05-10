#include "ImageClassifier.hpp"
#include "FAST/Data/Image.hpp"
#include "FAST/Algorithms/ImageResizer/ImageResizer.hpp"
#include "FAST/Algorithms/ScaleImage/ScaleImage.hpp"

namespace fast {

ImageClassifier::ImageClassifier() {
	createInputPort<Image>(0, true, INPUT_STATIC_OR_DYNAMIC, true);

	mModelLoaded = false;
}

// Get all available GPU devices
static void get_gpus(std::vector<int>* gpus) {
    int count = 0;
    count = caffe::Caffe::EnumerateDevices(true);
    for (int i = 0; i < count; ++i) {
      gpus->push_back(i);
    }
}


void ImageClassifier::loadModel(std::string modelFile, std::string trainingFile, std::string meanFile) {
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
	reportInfo() << "Loading model file.." << reportEnd();
	mNet = SharedPointer<caffe::Net<float> >(new caffe::Net<float>(modelFile, caffe::TEST, caffe::Caffe::GetDefaultDevice()));
	reportInfo() << "Finished loading model" << reportEnd();

	reportInfo() << "Loading training file.." << reportEnd();
	mNet->CopyTrainedLayersFrom(trainingFile);
	reportInfo() << "Finished loading training file." << reportEnd();

	if(mNet->num_inputs() != 1) {
		throw Exception("Number of inputs was not 1");
	}
	if(mNet->num_outputs() != 1) {
		throw Exception("Number of outputs was not 1");
	}

	reportInfo() << "Loading mean image file.." << reportEnd();
	caffe::BlobProto blob_proto;
	caffe::ReadProtoFromBinaryFileOrDie(meanFile.c_str(), &blob_proto);

	mMeanBlob.FromProto(blob_proto);
	reportInfo() << "Finished loading mean image file." << reportEnd();
	mModelLoaded = true;
}

void ImageClassifier::setLabels(std::vector<std::string> labels) {
	mResult.clear();
	mLabels.clear();
	mLabels = labels;
}

std::vector<std::map<std::string, float> > ImageClassifier::getResult() const {
	return mResult;
}

void ImageClassifier::execute() {
	if(!mModelLoaded)
		throw Exception("Model must be loaded in ImageClassifier before execution.");

	std::vector<Image::pointer> images = getMultipleStaticInputData<Image>();

	caffe::Blob<float>* input_layer = mNet->input_blobs()[0];
	if(input_layer->channels() != 1) {
		throw Exception("Number of input channels was not 1");
	}

	// nr of images x channels x width x height
	input_layer->Reshape(images.size(), 1, input_layer->width(), input_layer->height());
	mNet->Reshape();
	reportInfo() << "Input layer reshaped" << reportEnd();


	std::vector<Image::pointer> preProcessedImages;
	for(Image::pointer image : images) {
		// Resize image to fit input layer
		ImageResizer::pointer resizer = ImageResizer::New();
		resizer->setWidth(input_layer->width());
		resizer->setHeight(input_layer->height());
		resizer->setInputData(image);

		ScaleImage::pointer scale = ScaleImage::New();
		scale->setInputConnection(resizer->getOutputPort());
		scale->setHighestValue(255);
		scale->setLowestValue(0);
		scale->update();
		preProcessedImages.push_back(scale->getOutputData<Image>());
		reportInfo() << "Finished image resize and scale." << reportEnd();
	}

	// Set image to input layer
	int counter = 0;
	float* input_data = input_layer->mutable_cpu_data(); // This is the input data layer
	for(Image::pointer image : preProcessedImages) {
		ImageAccess::pointer access = image->getImageAccess(ACCESS_READ);
		float* pixels = (float*)access->get();
		Vector3ui size = image->getSize();
		float* meanPixels = mMeanBlob.mutable_cpu_data();
		for(int i = 0; i < size.x()*size.y(); ++i) {
			input_data[counter + i] = pixels[i] - meanPixels[i];
		}
		counter += size.x()*size.y();
	}

	// Do a forward pass
	mNet->Forward();

	// Read output layer
	caffe::Blob<float>* output_layer = mNet->output_blobs()[0];
	const float* begin = output_layer->cpu_data();
	const float* end = begin + output_layer->channels()*output_layer->num();
	std::vector<float> result(begin, end);
	//if(mLabels.size() != result.size())
	//	throw Exception("The number of labels did not match the number of predictions.");
	reportInfo() << "RESULT: " << reportEnd();
	mResult.clear();
	int k = 0;
	for(int i = 0; i < output_layer->num(); ++i) { // input images
		std::map<std::string, float> mapResult;
		for(int j = 0; j < output_layer->channels(); ++j) { // classes
			mapResult[mLabels[j]] = result[k];
			reportInfo() << mLabels[j] << ": " << result[k] << reportEnd();
			++k;
		}
		mResult.push_back(mapResult);
	}
}

}
