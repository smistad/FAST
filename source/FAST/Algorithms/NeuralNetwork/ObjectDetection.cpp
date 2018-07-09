#include "ObjectDetection.hpp"
#include "FAST/Data/Image.hpp"
#include "FAST/Data/PointSet.hpp"
#include "FAST/Algorithms/ImageResizer/ImageResizer.hpp"
#include "FAST/Data/Mesh.hpp"

namespace fast {

ObjectDetection::ObjectDetection() {
	createInputPort<Image>(0);
	createOutputPort<Mesh>(0);
	createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/NeuralNetwork/ObjectDetection.cl");

    //mOutputNames = {"Sigmoid", "Sigmoid_1", "Sigmoid_2"};
	mOutputNames = {"concat_v2"};
}

Vector2f applySpacing(Vector2f p, Vector3f spacing) {
	p.x() *= spacing.x();
	p.y() *= spacing.y();
	return p;
}

void ObjectDetection::execute() {
	Image::pointer image = getInputData<Image>();

	mRuntimeManager->startRegularTimer("resize_images");
    if(mWidth < 0 || mHeight < 0)
		throw Exception("Network input layer width and height has to be specified before running the network");

    // The size of the input layer is the target size
	// However, we want to preserve the aspect ratio, and cut the bottom or add zeros if it is too large or small
	// Resize, put preserve aspect ratio
    float scale = (float)mWidth / image->getWidth();
	int height = round(scale*image->getHeight());

	// Resize image to fit input layer
	ImageResizer::pointer resizer = ImageResizer::New();
	resizer->setWidth(mWidth); // This is the target width
	resizer->setHeight(height);
	resizer->setInputData(image);
	resizer->update();

	Image::pointer resizedImage = resizer->getOutputData<Image>();
	// The pre processed image is of the target size, and is also normalized
	Image::pointer preProcessedImageLeft = Image::New();
	preProcessedImageLeft->create(mWidth, mHeight, TYPE_FLOAT, 1);
	OpenCLDevice::pointer device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
	cl::Program program = getOpenCLProgram(device);
	cl::Kernel normalizationKernel(program, "imageNormalization");
	{
		OpenCLImageAccess::pointer access = resizedImage->getOpenCLImageAccess(ACCESS_READ, device);
		OpenCLImageAccess::pointer accessLeft = preProcessedImageLeft->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
		normalizationKernel.setArg(0, *(access->get2DImage()));
		normalizationKernel.setArg(1, *(accessLeft->get2DImage()));

		device->getCommandQueue().enqueueNDRangeKernel(
				normalizationKernel,
				cl::NullRange,
				cl::NDRange(mWidth, mHeight),
				cl::NullRange
		);
		device->getCommandQueue().finish();
	}
	mRuntimeManager->stopRegularTimer("resize_images");

	executeNetwork({preProcessedImageLeft});

	mRuntimeManager->startRegularTimer("create_mesh");
    // Get outputs
	std::vector<float> result = getNetworkOutput("concat_v2")[0];
	std::vector<float> detectorResult(result.begin(), result.begin()+5);
	std::vector<float> positionResult(result.begin()+5, result.begin()+15);
	std::vector<float> sizeResult(result.begin()+15, result.end());
	//std::vector<float> detectorResult = getNetworkOutput("Sigmoid")[0];
	//std::vector<float> positionResult = getNetworkOutput("Sigmoid_1")[0];
	//std::vector<float> sizeResult = getNetworkOutput("Sigmoid_2")[0];

	std::vector<MeshVertex> vertices;
	std::vector<VectorXui> lines;
	Mesh::pointer mesh = getOutputData<Mesh>();


    const int nbObjects = detectorResult.size();
    int counter = 0;
    bool mDrawBoxes = false;

    for(int objectID = 0; objectID < nbObjects; ++objectID) {
		std::cout << detectorResult[objectID] << std::endl;
		if(detectorResult[objectID] < 0.9)
			continue;

        float width = image->getWidth();
        float height = image->getHeight();


        // Convert from normalized coordinates
        std::cout << positionResult[objectID*2] << " " << positionResult[objectID*2+1] << std::endl;
        Vector2f center((positionResult[objectID*2] * mWidth) / scale,
                        positionResult[1+objectID*2] * mHeight / scale);
		std::cout << center.transpose() << std::endl;
		std::cout << sizeResult[objectID*2] << " " << sizeResult[objectID*2+1] << std::endl;

		Vector2f corner1 = center;
		Vector2f corner2 = center;
		Vector2f corner3 = center;
		Vector2f corner4 = center;
        if(mDrawBoxes) {
			float bboxWidth = sizeResult[objectID * 2] * mWidth / scale;
			float bboxHeight = sizeResult[1 + objectID * 2] * mHeight / scale;
			std::cout << bboxWidth << " " << bboxHeight << std::endl;

			corner1.x() -= bboxWidth * 0.5;
			corner1.y() -= bboxHeight * 0.5;
			corner2.x() -= bboxWidth * 0.5;
			corner2.y() += bboxHeight * 0.5;
			corner3.x() += bboxWidth * 0.5;
			corner3.y() += bboxHeight * 0.5;
			corner4.x() += bboxWidth * 0.5;
			corner4.y() -= bboxHeight * 0.5;
		} else {
			corner1.x() -= 5;
			corner2.x() += 5;
			corner3.y() -= 5;
			corner4.y() += 5;
		}
		corner1 = applySpacing(corner1, image->getSpacing());
		corner2 = applySpacing(corner2, image->getSpacing());
		corner3 = applySpacing(corner3, image->getSpacing());
		corner4 = applySpacing(corner4, image->getSpacing());

        MeshVertex vertex = MeshVertex(corner1);
		vertex.setLabel(objectID+1);
        vertices.push_back(vertex);
		vertex = MeshVertex(corner2);
		vertex.setLabel(objectID+1);
		vertices.push_back(vertex);
		vertex = MeshVertex(corner3);
		vertex.setLabel(objectID+1);
		vertices.push_back(vertex);
		vertex = MeshVertex(corner4);
		vertex.setLabel(objectID+1);
		vertices.push_back(vertex);

        if(mDrawBoxes) {
			lines.push_back(Vector2ui(0 + counter * 4, 1 + counter * 4));
			lines.push_back(Vector2ui(1 + counter * 4, 2 + counter * 4));
			lines.push_back(Vector2ui(2 + counter * 4, 3 + counter * 4));
			lines.push_back(Vector2ui(3 + counter * 4, 0 + counter * 4));
		} else {
			lines.push_back(Vector2ui(0 + counter * 4, 1 + counter * 4));
			lines.push_back(Vector2ui(2 + counter * 4, 3 + counter * 4));
		}
        counter++;
	}

    mesh->create(vertices, lines);
	mRuntimeManager->stopRegularTimer("create_mesh");

    /*
	OpenCLDevice::pointer device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
	cl::Program program = getOpenCLProgram(device);
	cl::Kernel normalizationKernel(program, "imageNormalization");

	// TODO resize image to 256 256 first, then crop into left and right
    int target_width = 256;

	// The size of the input layer is the target size
	// However, we want to preserve the aspect ratio, and cut the bottom or add zeros if it is too large or small
	// Resize, put preserve aspect ratio
	float scale = (float)target_width / image->getWidth();
	int height = round(scale*image->getHeight());
	reportInfo() << "Target size: " << target_width << " " << input_layer->height() << reportEnd();

	// Resize image to fit input layer
	ImageResizer::pointer resizer = ImageResizer::New();
	resizer->setWidth(target_width); // This is the target width
	resizer->setHeight(height);
	resizer->setInputData(image);
	resizer->update();

	Image::pointer resizedImage = resizer->getOutputData<Image>();
	// The pre processed image is of the target size, and is also normalized
	Image::pointer preProcessedImageLeft = Image::New();
	preProcessedImageLeft->create(input_layer->width(), input_layer->height(), TYPE_FLOAT, 1);
	Image::pointer preProcessedImageRight = Image::New();
	preProcessedImageRight->create(input_layer->width(), input_layer->height(), TYPE_FLOAT, 1);
	{
		OpenCLImageAccess::pointer access = resizedImage->getOpenCLImageAccess(ACCESS_READ, device);
		OpenCLImageAccess::pointer accessLeft = preProcessedImageLeft->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
		OpenCLImageAccess::pointer accessRight = preProcessedImageRight->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
		normalizationKernel.setArg(0, *(access->get2DImage()));
		normalizationKernel.setArg(1, *(accessLeft->get2DImage()));
		normalizationKernel.setArg(2, *(accessRight->get2DImage()));

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
	ImageAccess::pointer accessLeft = preProcessedImageLeft->getImageAccess(ACCESS_READ);
	ImageAccess::pointer accessRight = preProcessedImageRight->getImageAccess(ACCESS_READ);
	float* pixels = (float*)accessLeft->get();
	Vector3ui size = preProcessedImageLeft->getSize();
	// In a 4D blob, the value at index (0, 0, h, w) is physically located at index (h) * W + w., which is y*width + x, same as FAST
	for(int i = 0; i < size.x()*size.y(); ++i) {
		input_data[i] = pixels[i];
	}
	pixels = (float*)accessRight->get();
	for(int i = 0; i < size.x()*size.y(); ++i) {
		input_data[size.x()*size.y() + i] = pixels[i];
	}

	// Do a forward pass
	mNetwork->Forward();

	// TODO fix this for left and right:

	// Read outputs
	std::vector<float> softmax_result = getNetworkOutput("softmax");
	std::vector<float> center_result = getNetworkOutput("center_fc_final");
	std::vector<float> size_result = getNetworkOutput("size_fc_final");

	std::vector<MeshVertex> vertices;
	std::vector<VectorXui> lines;
	Mesh::pointer mesh = getOutputData<Mesh>();

    bool found_vessel = false;
	for(int i = 0; i < 2; ++i) { // For left and right
        int delta_x = 0;
		if(i == 1)
			delta_x = 256-192;
		if(softmax_result[1+2*i] > 0.9) {
            if(found_vessel) {
				// Remove the one found on left side, only use right side
				vertices.clear();
				lines.clear();
			}
            if(i == 0) {
				std::cout << "Found vessel on LEFT side" << std::endl;
			} else {
				std::cout << "Found vessel on RIGHT side" << std::endl;
			}

			//if(mLabels.size() != result.size())
			//	throw Exception("The number of labels did not match the number of predictions.");
			float width = image->getWidth();
			height = image->getHeight();


			// Convert from normalized coordinates
			Vector2f center((center_result[0+2*i] * input_layer->width() + delta_x) / scale,
							center_result[1+2*i] * input_layer->height() / scale);

			float bboxWidth = size_result[0+2*i] * target_width / scale;
			float bboxHeight = size_result[1+2*i] * input_layer->height() / scale;

			Vector2f corner1 = center;
			corner1.x() -= bboxWidth * 0.5;
			corner1.y() -= bboxHeight * 0.5;
			corner1 = applySpacing(corner1, image->getSpacing());
			Vector2f corner2 = center;
			corner2.x() -= bboxWidth * 0.5;
			corner2.y() += bboxHeight * 0.5;
			corner2 = applySpacing(corner2, image->getSpacing());
			Vector2f corner3 = center;
			corner3.x() += bboxWidth * 0.5;
			corner3.y() += bboxHeight * 0.5;
			corner3 = applySpacing(corner3, image->getSpacing());
			Vector2f corner4 = center;
			corner4.x() += bboxWidth * 0.5;
			corner4.y() -= bboxHeight * 0.5;
			corner4 = applySpacing(corner4, image->getSpacing());

			vertices.push_back(MeshVertex(corner1));
			vertices.push_back(MeshVertex(corner2));
			vertices.push_back(MeshVertex(corner3));
			vertices.push_back(MeshVertex(corner4));

			lines.push_back(Vector2ui(0, 1));
			lines.push_back(Vector2ui(1, 2));
			lines.push_back(Vector2ui(2, 3));
			lines.push_back(Vector2ui(3, 0));
            found_vessel = true;
		}
	}

	mesh->create(vertices, lines);
     */
}



}
