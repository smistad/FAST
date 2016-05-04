#include "ImageClassifier.hpp"
#include "FAST/Data/Image.hpp"
#include <caffe/caffe.hpp>

namespace fast {

ImageClassifier::ImageClassifier() {
	createInputPort<Image>(0);
}

// Get all available GPU devices
static void get_gpus(std::vector<int>* gpus) {
    int count = 0;
    count = caffe::Caffe::EnumerateDevices(true);
    for (int i = 0; i < count; ++i) {
      gpus->push_back(i);
    }
}


static void subtractMeanFromImage(float* pixels, Vector3ui size, std::string meanFile) {
	caffe::BlobProto blob_proto;
	caffe::ReadProtoFromBinaryFileOrDie(meanFile.c_str(), &blob_proto);

	/* Convert from BlobProto to Blob<float> */
	caffe::Blob<float> mean_blob;
	mean_blob.FromProto(blob_proto);
	float* meanPixels = mean_blob.mutable_cpu_data();
	for(int i = 0; i < size.x()*size.y(); ++i) {
		pixels[i] = round(pixels[i]*255);
		pixels[i] -= meanPixels[i];
	}
}

void ImageClassifier::execute() {
	Image::pointer image = getStaticInputData<Image>();

	std::vector<int> gpus;
	get_gpus(&gpus);
	if (gpus.size() != 0) {
		reportInfo() << "Use GPU with device ID " << gpus[0] << reportEnd();
		caffe::Caffe::SetDevices(gpus);
		caffe::Caffe::set_mode(caffe::Caffe::GPU);
		caffe::Caffe::SetDevice(gpus[0]);
	}
	//caffe::Caffe::set_mode(caffe::Caffe::CPU);

	std::string modelFile = "/home/smistad/workspace/caffe-test/source/models/vessel_ultrasound_lenet/deploy.prototxt";
	std::string trainingFile = "/home/smistad/workspace/caffe-test/source/models/vessel_ultrasound_lenet/snapshot_iter_90.caffemodel";
	std::string file = "/home/smistad/workspace/caffe-test/source/models/vessel_ultrasound_lenet/image.png";
	std::string meanFile = "/home/smistad/workspace/caffe-test/source/models/vessel_ultrasound_lenet/mean.binaryproto";

	reportInfo() << "Loading model file.." << reportEnd();
	boost::shared_ptr<caffe::Net<float> > net(new caffe::Net<float>(modelFile, caffe::TEST, caffe::Caffe::GetDefaultDevice()));
	reportInfo() << "Finished loading model" << reportEnd();

	reportInfo() << "Loading training file.." << reportEnd();
	net->CopyTrainedLayersFrom(trainingFile);
	reportInfo() << "Finished loading training file." << reportEnd();

	if(net->num_inputs() != 1) {
		throw Exception("Number of inputs was not 1");
	}
	if(net->num_outputs() != 1) {
		throw Exception("Number of outputs was not 1");
	}
	if(image->getDataType() != TYPE_FLOAT) {
		throw Exception("Only float images supported currently");
	}

	caffe::Blob<float>* input_layer = net->input_blobs()[0];
	if(input_layer->channels() != 1) {
		throw Exception("Number of input channels was not 1");
	}

	input_layer->Reshape(1, 1, 64, 64);
	/* Forward dimension change to all layers. */
	net->Reshape();
	std::cout << "Input layer reshaped" << std::endl;

	// TODO Resize image to fit input layer
	// TODO Load mean image and subtract from image
	// TODO convert to float
	// TODO Set image to input layer
	float* input_data = input_layer->mutable_cpu_data(); // This is the input data
	ImageAccess::pointer access = image->getImageAccess(ACCESS_READ);
	float* image_data = (float*)access->get();
	subtractMeanFromImage(image_data, image->getSize(), meanFile);
	memcpy(input_data, image_data, sizeof(float)*image->getWidth()*image->getHeight());

	// Do a forward pass
	net->Forward();

	// Read output layer
	caffe::Blob<float>* output_layer = net->output_blobs()[0];
	const float* begin = output_layer->cpu_data();
	const float* end = begin + output_layer->channels();
	std::vector<float> result(begin, end);
	std::cout << "RESULT: " << std::endl;
	std::cout << "Not vessel: " << result[0] << std::endl;
	std::cout << "Vessel: " << result[1] << std::endl;
}

}
