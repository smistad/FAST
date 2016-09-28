#include "ObjectDetection.hpp"
#include "FAST/Data/Image.hpp"
#include "FAST/Data/PointSet.hpp"
#include "FAST/Algorithms/ImageResizer/ImageResizer.hpp"
#include "FAST/Data/Mesh.hpp"
#include <boost/algorithm/string.hpp>

namespace fast {

ObjectDetection::ObjectDetection() {
	createInputPort<Image>(0);
	createOutputPort<Mesh>(0, OUTPUT_DEPENDS_ON_INPUT, 0);
	createOpenCLProgram(std::string(FAST_SOURCE_DIR) + "Algorithms/ObjectDetection/ObjectDetection.cl");
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

void ObjectDetection::loadModel(std::string modelFile, std::string trainingFile) {
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

	mModelLoaded = true;
}

Vector2f applySpacing(Vector2f p, Vector3f spacing) {
	p.x() *= spacing.x();
	p.y() *= spacing.y();
	return p;
}

void ObjectDetection::execute() {
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
	for(int i = 0; i < size.x()*size.y(); ++i) {
		input_data[i] = pixels[i];
	}

	// Do a forward pass
	mNet->Forward();

	// Read output layer
	boost::shared_ptr<caffe::Blob<float> > softmax_layer = mNet->blob_by_name("softmax");
	std::vector<float> softmax_result(softmax_layer->cpu_data(), softmax_layer->cpu_data() + softmax_layer->channels()*softmax_layer->num());

	std::vector<MeshVertex> vertices;
	std::vector<VectorXui> lines;
	Mesh::pointer mesh = getStaticOutputData<Mesh>();
	if(softmax_result[1] > 0.9) {
		boost::shared_ptr<caffe::Blob<float> > center_layer = mNet->blob_by_name("center_fc");
		std::vector<float> center_result(center_layer->cpu_data(), center_layer->cpu_data() + center_layer->channels()*center_layer->num());
		boost::shared_ptr<caffe::Blob<float> > size_layer = mNet->blob_by_name("size_fc");
		std::vector<float> size_result(size_layer->cpu_data(), size_layer->cpu_data() + size_layer->channels()*size_layer->num());
		//if(mLabels.size() != result.size())
		//	throw Exception("The number of labels did not match the number of predictions.");
		float width = image->getWidth();
		height = image->getHeight();


		// Convert from normalized coordinates
		Vector2f center(center_result[0]*input_layer->width()/scale, center_result[1]*input_layer->height()/scale);

		float bboxWidth = size_result[0]*input_layer->width()/scale;
		float bboxHeight = size_result[1]*input_layer->height()/scale;

		Vector2f corner1 = center;
		corner1.x() -= bboxWidth*0.5;
		corner1.y() -= bboxHeight*0.5;
		corner1 = applySpacing(corner1, image->getSpacing());
		Vector2f corner2 = center;
		corner2.x() -= bboxWidth*0.5;
		corner2.y() += bboxHeight*0.5;
		corner2 = applySpacing(corner2, image->getSpacing());
		Vector2f corner3 = center;
		corner3.x() += bboxWidth*0.5;
		corner3.y() += bboxHeight*0.5;
		corner3 = applySpacing(corner3, image->getSpacing());
		Vector2f corner4 = center;
		corner4.x() += bboxWidth*0.5;
		corner4.y() -= bboxHeight*0.5;
		corner4 = applySpacing(corner4, image->getSpacing());

		vertices.push_back(MeshVertex(corner1));
		vertices.push_back(MeshVertex(corner2));
		vertices.push_back(MeshVertex(corner3));
		vertices.push_back(MeshVertex(corner4));

		lines.push_back(Vector2ui(0, 1));
		lines.push_back(Vector2ui(1, 2));
		lines.push_back(Vector2ui(2, 3));
		lines.push_back(Vector2ui(3, 0));
	}

	mesh->create(vertices, lines);
}



}
