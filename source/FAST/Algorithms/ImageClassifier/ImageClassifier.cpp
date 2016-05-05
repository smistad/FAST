#include "ImageClassifier.hpp"
#include "FAST/Data/Image.hpp"
#include "FAST/Algorithms/ImageResizer/ImageResizer.hpp"

namespace fast {

ImageClassifier::ImageClassifier() {
	createInputPort<Image>(0);

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
	/*
	get_gpus(&gpus);
	if (gpus.size() != 0) {
		reportInfo() << "Use GPU with device ID " << gpus[0] << reportEnd();
		caffe::Caffe::SetDevices(gpus);
		caffe::Caffe::set_mode(caffe::Caffe::GPU);
		caffe::Caffe::SetDevice(gpus[0]);
	}
	*/
	caffe::Caffe::set_mode(caffe::Caffe::CPU);
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

std::map<std::string, float> ImageClassifier::getResult() const {
	return mResult;
}

void ImageClassifier::execute() {
	if(!mModelLoaded)
		throw Exception("Model must be loaded in ImageClassifier before execution.");

	Image::pointer image = getStaticInputData<Image>();
	if(image->getDataType() != TYPE_FLOAT) {
		throw Exception("Only float images supported currently");
	}

	caffe::Blob<float>* input_layer = mNet->input_blobs()[0];
	if(input_layer->channels() != 1) {
		throw Exception("Number of input channels was not 1");
	}

	/*
	// What is the purpose of this??
	input_layer->Reshape(1, 1, 64, 64);
	mNet->Reshape();
	std::cout << "Input layer reshaped" << std::endl;
	*/

	// Resize image to fit input layer
	ImageResizer::pointer resizer = ImageResizer::New();
	resizer->setWidth(input_layer->width());
	resizer->setHeight(input_layer->height());
	resizer->setInputData(image);
	resizer->update();
	image = resizer->getOutputData<Image>();

	// TODO Load mean image and subtract from image
	// TODO convert to float
	// TODO Set image to input layer
	float* input_data = input_layer->mutable_cpu_data(); // This is the input data
	ImageAccess::pointer access = image->getImageAccess(ACCESS_READ);
	float* pixels = (float*)access->get();
	Vector3ui size = image->getSize();
	float* meanPixels = mMeanBlob.mutable_cpu_data();
	for(int i = 0; i < size.x()*size.y(); ++i) {
		pixels[i] = round(pixels[i]*255);
		pixels[i] -= meanPixels[i];
	}
	memcpy(input_data, pixels, sizeof(float)*image->getWidth()*image->getHeight());

	// Do a forward pass
	mNet->Forward();

	// Read output layer
	caffe::Blob<float>* output_layer = mNet->output_blobs()[0];
	const float* begin = output_layer->cpu_data();
	const float* end = begin + output_layer->channels();
	std::vector<float> result(begin, end);
	if(mLabels.size() != result.size())
		throw Exception("The number of labels did not match the number of predictions.");
	reportInfo() << "RESULT: " << reportEnd();
	for(int i = 0; i < result.size(); ++i) {
		mResult[mLabels[i]] = result[i];
		reportInfo() << mLabels[i] << ": " << result[i] << reportEnd();
	}
}

}
