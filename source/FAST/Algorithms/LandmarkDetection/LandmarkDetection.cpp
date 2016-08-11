#include "LandmarkDetection.hpp"
#include "FAST/Data/Image.hpp"
#include "FAST/Data/PointSet.hpp"
#include "FAST/Algorithms/ImageResizer/ImageResizer.hpp"

namespace fast {

LandmarkDetection::LandmarkDetection() {
	createInputPort<Image>(0);
	createOutputPort<PointSet>(0, OUTPUT_DEPENDS_ON_INPUT, 0);
	createOpenCLProgram(std::string(FAST_SOURCE_DIR) + "Algorithms/LandmarkDetection/LandmarkDetection.cl");
	mModelLoaded = false;
	mMirrorImage = false;
}

void LandmarkDetection::setMirrorImage(bool mirrorImage) {
	mMirrorImage = mirrorImage;
}

// Get all available GPU devices
static void get_gpus(std::vector<int>* gpus) {
    int count = 0;
    count = caffe::Caffe::EnumerateDevices(true);
    for (int i = 0; i < count; ++i) {
      gpus->push_back(i);
    }
}

void LandmarkDetection::loadModel(std::string modelFile, std::string trainingFile) {
	std::vector<int> gpus;
	get_gpus(&gpus);
	if (gpus.size() != 0) {
		reportInfo() << "Use OpenCL device with ID " << gpus[0] << reportEnd();
		caffe::Caffe::SetDevices(gpus);
		caffe::Caffe::set_mode(caffe::Caffe::GPU);
		caffe::Caffe::SetDevice(gpus[0]);
	}
	//FLAGS_minloglevel = 5; // Disable cout from caffe
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

	mModelLoaded = true;
}

void LandmarkDetection::execute() {
	if(!mModelLoaded)
		throw Exception("Model must be loaded in ImageClassifier before execution.");

	Image::pointer image = getStaticInputData<Image>();

	caffe::Blob<float>* input_layer = mNet->input_blobs()[0];
	if(input_layer->channels() != 1) {
		throw Exception("Number of input channels was not 1");
	}

	// nr of images x channels x width x height
	input_layer->Reshape(1, 1, input_layer->height(), input_layer->width());
	mNet->Reshape();
	reportInfo() << "Net reshaped" << reportEnd();

	OpenCLDevice::pointer device = getMainDevice();
	cl::Program program = getOpenCLProgram(device);
	cl::Kernel normalizationKernel(program, "imageNormalization");

	// The size of the input layer is the target size
	// However, we want to preserve the aspect ratio, and cut the bottom or add zeros if it is too large or small
	// Resize, put preserve aspect ratio
	float scale = (float)input_layer->width() / image->getWidth();
	int height = round(scale*image->getHeight());
	reportInfo() << "Target size: " << input_layer->width() << " " << input_layer->height() << reportEnd();

	// Resize image to fit input layer
	ImageResizer::pointer resizer = ImageResizer::New();
	resizer->setWidth(input_layer->width()); // This is the target width
	resizer->setHeight(height);
	resizer->setInputData(image);
	resizer->update();
	Image::pointer resizedImage = resizer->getOutputData<Image>();
	// The pre processed image is of the target size, and is also normalized
	Image::pointer preProcessedImage = Image::New();
	preProcessedImage->create(input_layer->width(), input_layer->height(), TYPE_FLOAT, 1);
	{
		OpenCLImageAccess::pointer access = resizedImage->getOpenCLImageAccess(ACCESS_READ, device);
		OpenCLImageAccess::pointer access2 = preProcessedImage->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
		normalizationKernel.setArg(0, *(access->get2DImage()));
		normalizationKernel.setArg(1, *(access2->get2DImage()));

		device->getCommandQueue().enqueueNDRangeKernel(
				normalizationKernel,
				cl::NullRange,
				cl::NDRange(input_layer->width(), input_layer->height()),
				cl::NullRange
		);
		device->getCommandQueue().finish();
	}

	reportInfo() << "Finished image resize and normalization." << reportEnd();

	// Set image to input layer
	float* input_data = input_layer->mutable_cpu_data(); // This is the input data layer
	ImageAccess::pointer access = preProcessedImage->getImageAccess(ACCESS_READ);
	float* pixels = (float*)access->get();
	Vector3ui size = preProcessedImage->getSize();
	// In a 4D blob, the value at index (0, 0, h, w) is physically located at index (h) * W + w., which is y*width + x, same as FAST
	if(mMirrorImage) {
		for(int y = 0; y < size.y(); ++y) {
		for(int x = 0; x < size.x(); ++x) {
			input_data[x + y*size.x()] = pixels[(size.x() - x - 1) + y*size.x()];
		}}
	} else {
		for(int i = 0; i < size.x()*size.y(); ++i) {
			input_data[i] = pixels[i];
		}
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
	std::vector<Vector3f> points;
	float width = image->getWidth();
	height = image->getHeight();

	for(int i = 0; i < result.size(); i += 2) {
		// Convert from normalized coordinates
		if(mMirrorImage)
			result[i] *= -1;
		Vector3f landmark((result[i]*height + width*0.5f), (result[i+1]*height + width*0.5f), 0);
		// Convert to mm
		landmark.x() *= image->getSpacing().x();
		landmark.y() *= image->getSpacing().y();
		points.push_back(landmark);
	}

	PointSet::pointer output = getStaticOutputData<PointSet>();
	output->create(points);

}

}
