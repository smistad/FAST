#include "DNNAppearanceModel.hpp"
#include "FAST/Data/Image.hpp"
#include "FAST/Data/PointSet.hpp"
#include "FAST/Algorithms/ImageResizer/ImageResizer.hpp"
#include "FAST/Data/Mesh.hpp"
#include "FAST/Utility.hpp"

namespace fast {

DNNAppearanceModel::DNNAppearanceModel() {
	createInputPort<Image>(0);
	createOutputPort<Mesh>(0, OUTPUT_DEPENDS_ON_INPUT, 0);
	createOpenCLProgram(std::string(FAST_SOURCE_DIR) + "Algorithms/NeuralNetwork/DNNAppearanceModel.cl");
	mMirrorImage = false;
	mObjectsLoaded = false;
}

void DNNAppearanceModel::setMirrorImage(bool mirrorImage) {
	mMirrorImage = mirrorImage;
}

void DNNAppearanceModel::loadObjects(std::string objectsFile) {
	reportInfo() << "Loading objects file.." << reportEnd();
	mOutputMesh = Mesh::New();
	std::vector<MeshVertex> vertices;
	std::vector<VectorXui> connections;

	std::ifstream file(objectsFile);
	int vertexCount = 0;
	while(!file.eof()) {
		std::string line;
		std::getline(file, line);

		std::vector<std::string> tokens = split(line);

		if(tokens[0] == "object") {
			int objectLabel = std::stoi(tokens[1]);
			int nrOfVertices = std::stoi(tokens[2]);
			int nrOfLines = std::stoi(tokens[3]);

			// Create vertices
			for(int i = 0; i < nrOfVertices; ++i) {
				MeshVertex vertex(Vector2f::Zero());
				vertex.setLabel(objectLabel);
				vertices.push_back(vertex);
			}

			// Get lines
			for(int i = 0; i < nrOfLines; ++i) {
				std::getline(file, line);
				std::vector<std::string> indices = split(line);
				Vector2ui connection(vertexCount + std::stoi(indices[0]), vertexCount + std::stoi(indices[1]));
				connections.push_back(connection);
			}
			vertexCount += nrOfVertices;
		}
	}

	mOutputMesh->create(vertices, connections);
	reportInfo() << "Finished loading objects file." << reportEnd();
    mObjectsLoaded = true;
}

void DNNAppearanceModel::execute() {
	if(!mModelLoaded)
		throw Exception("Model must be loaded before execution.");
	if(!mObjectsLoaded)
		throw Exception("Objects must be loaded before execution.");

	Image::pointer image = getStaticInputData<Image>();

	caffe::Blob<float>* input_layer = mNetwork->input_blobs()[0];
	if(input_layer->channels() != 1) {
		throw Exception("Number of input channels was not 1");
	}

	// nr of images x channels x width x height
	input_layer->Reshape(1, 1, input_layer->height(), input_layer->width());
	mNetwork->Reshape();
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
	mNetwork->Forward();

	// Read output layer
	std::vector<float> result = getNetworkOutput();
	//if(mLabels.size() != result.size())
	//	throw Exception("The number of labels did not match the number of predictions.");
	float width = image->getWidth();
	height = image->getHeight();

	MeshAccess::pointer meshAccess = mOutputMesh->getMeshAccess(ACCESS_READ_WRITE);
	int j = 0;
	for(int i = 0; i < result.size(); i += 2) {
		// Convert from normalized coordinates
		if(mMirrorImage)
			result[i] *= -1;
		Vector2f landmark((result[i]*height + width*0.5f), (result[i+1]*height + width*0.5f));
		// Convert to mm
		landmark.x() *= image->getSpacing().x();
		landmark.y() *= image->getSpacing().y();

		MeshVertex vertex = meshAccess->getVertex(j);
		vertex.setPosition(landmark);
		meshAccess->setVertex(j, vertex);
		j++;
	}

	setStaticOutputData<Mesh>(0, mOutputMesh);

}

}
