#include "FAST/Testing.hpp"
#include "FAST/Streamers/ImageFileStreamer.hpp"
#include "KalmanFilter.hpp"
#include "FAST/Visualization/MeshRenderer/MeshRenderer.hpp"
#include "FAST/Visualization/SliceRenderer/SliceRenderer.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"
#include "FAST/Algorithms/ImageSlicer/ImageSlicer.hpp"
#include "AppearanceModels/StepEdge/StepEdgeModel.hpp"
#include "ShapeModels/MeanValueCoordinates/MeanValueCoordinatesModel.hpp"
#include "ShapeModels/Ellipse/EllipseModel.hpp"
#include "ShapeModels/CardinalSpline/CardinalSplineModel.hpp"

using namespace fast;

/*
TEST_CASE("Model based segmentation with mean value coordinates on 3D cardiac US data", "[fast][ModelBasedSegmentation][cardiac][3d][visual]") {
	ImageFileStreamer::pointer streamer = ImageFileStreamer::New();
	streamer->setFilenameFormat("/home/smistad/CETUS/Patient1/Patient1_frame#.mhd");
	streamer->setZeroFilling(2);
	streamer->setStartNumber(1);
	streamer->enableLooping();
	streamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
	//streamer->setSleepTime(2000);

	MeanValueCoordinatesModel::pointer shapeModel = MeanValueCoordinatesModel::New();
	shapeModel->loadMeshes("/home/smistad/Dropbox/Programmering/Mean value coordinates/cetus_model_mesh_small.vtk",
			"/home/smistad/Dropbox/Programmering/Mean value coordinates/cetus_control_mesh.vtk");
	shapeModel->initializeShapeToImageCenter();
	KalmanFilter::pointer segmentation = KalmanFilter::New();
	StepEdgeModel::pointer appearanceModel = StepEdgeModel::New();
	appearanceModel->setLineLength(0.025);
	appearanceModel->setLineSampleSpacing(0.025/32.0);
	segmentation->setAppearanceModel(appearanceModel);
	segmentation->setShapeModel(shapeModel);
	segmentation->setInputConnection(streamer->getOutputPort());

	MeshRenderer::pointer meshRenderer = MeshRenderer::New();
	meshRenderer->setInputConnection(segmentation->getOutputPort());

	SliceRenderer::pointer sliceRenderer = SliceRenderer::New();
	sliceRenderer->setInputConnection(streamer->getOutputPort());
	sliceRenderer->setSlicePlane(PLANE_Y);

	SimpleWindow::pointer window = SimpleWindow::New();
	window->addRenderer(meshRenderer);
	window->addRenderer(sliceRenderer);
	window->start();
}

TEST_CASE("Model based segmentation with spline model on 2D pediatric cardiac US data", "[fast][ModelBasedSegmentation][2d][cardiac][pediatric][visual]") {
	ImageFileStreamer::pointer streamer = ImageFileStreamer::New();
	streamer->setFilenameFormat("/home/smistad/Cardiac_2D/test2/labelImage#.mhd");
	streamer->setZeroFilling(2);
	streamer->enableLooping();
	streamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
	streamer->update(); // TODO this should not be needed
	streamer->setSleepTime(500);

	// Control points for spline model
	std::vector<Vector2f> controlPoints = {
			Vector2f(2.822184520427274634e+01, 2.204189742133431196e+01),
			Vector2f(2.805852557576601924e+01, 3.001778981203989716e+01),
			Vector2f(2.981067073476717511e+01, 4.219496931887970703e+01),
			Vector2f(4.537380221088420029e+01, 4.078267995844019822e+01),
			Vector2f(4.447638939087526921e+01, 2.948818130187507691e+01),
			Vector2f(3.884196391885149779e+01, 2.151228891116949882e+01)
	};

	CardinalSplineModel::pointer shapeModel = CardinalSplineModel::New();
	shapeModel->setControlPoints(controlPoints);
	shapeModel->setGlobalProcessError(0.000001f);
	shapeModel->setLocalProcessError(0.0000001f);
	shapeModel->setResolution(12);
	shapeModel->setScalingLimit(0.5);
	//shapeModel->setTension({0, 0.75, 0.75, 0, 0, 0});
	KalmanFilter::pointer segmentation = KalmanFilter::New();
	StepEdgeModel::pointer appearanceModel = StepEdgeModel::New();
	appearanceModel->setLineLength(6);
	appearanceModel->setLineSampleSpacing(6/32.0);
	appearanceModel->setIntensityDifferenceThreshold(20);
	appearanceModel->setEdgeType(StepEdgeModel::EDGE_TYPE_BLACK_INSIDE_WHITE_OUTSIDE);
	appearanceModel->setMinimumDepth(10);
	segmentation->setStartIterations(10);
	segmentation->setIterations(10);
	segmentation->setAppearanceModel(appearanceModel);
	segmentation->setShapeModel(shapeModel);
	segmentation->setInputConnection(streamer->getOutputPort());

	MeshRenderer::pointer meshRenderer = MeshRenderer::New();
	meshRenderer->addInputConnection(segmentation->getOutputPort());
	meshRenderer->addInputConnection(segmentation->getDisplacementsOutputPort(), Color::Red(), 1.0);

	ImageRenderer::pointer imageRenderer = ImageRenderer::New();
	imageRenderer->addInputConnection(streamer->getOutputPort());

	SimpleWindow::pointer window = SimpleWindow::New();
	window->addRenderer(imageRenderer);
	window->addRenderer(meshRenderer);
	window->set2DMode();
	window->start();
}

TEST_CASE("Model based segmentation with spline model on 2D pediatric aorta US data", "[fast][ModelBasedSegmentation][2d][aorta][pediatric][visual]") {
	ImageFileStreamer::pointer streamer = ImageFileStreamer::New();
	streamer->setFilenameFormat("/home/smistad/Data/aorta_us/labelImage#.mhd");
	streamer->setZeroFilling(2);
	streamer->enableLooping();
	streamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
	streamer->update(); // TODO this should not be needed
	streamer->setSleepTime(1000);

	// Control points for spline model
	std::vector<Vector2f> controlPoints = {
	        Vector2f(35.0, 45.1),
	        Vector2f(35.0, 55.1),
	        Vector2f(50.0, 55.1),
	        Vector2f(50.0, 45.1),
	};

	CardinalSplineModel::pointer shapeModel = CardinalSplineModel::New();
	shapeModel->setControlPoints(controlPoints);
	shapeModel->setGlobalProcessError(0.0001f);
	shapeModel->setLocalProcessError(0.00001f);
	shapeModel->setResolution(12);
	KalmanFilter::pointer segmentation = KalmanFilter::New();
	StepEdgeModel::pointer appearanceModel = StepEdgeModel::New();
	appearanceModel->setLineLength(8);
	appearanceModel->setLineSampleSpacing(8/32.0);
	appearanceModel->setIntensityDifferenceThreshold(10);
	appearanceModel->setMinimumDepth(15);
	segmentation->setStartIterations(10);
	segmentation->setIterations(10);
	segmentation->setAppearanceModel(appearanceModel);
	segmentation->setShapeModel(shapeModel);
	segmentation->setInputConnection(streamer->getOutputPort());

	MeshRenderer::pointer meshRenderer = MeshRenderer::New();
	meshRenderer->addInputConnection(segmentation->getOutputPort());
	meshRenderer->addInputConnection(segmentation->getDisplacementsOutputPort(), Color::Red(), 1.0);

	ImageRenderer::pointer imageRenderer = ImageRenderer::New();
	imageRenderer->addInputConnection(streamer->getOutputPort());

	SimpleWindow::pointer window = SimpleWindow::New();
	window->addRenderer(imageRenderer);
	window->addRenderer(meshRenderer);
	window->set2DMode();
	window->start();
}

TEST_CASE("Model based segmentation with spline model on 2D cardiac US data", "[fast][ModelBasedSegmentation][2d][cardiac][visual]") {
	ImageFileStreamer::pointer streamer = ImageFileStreamer::New();
	streamer->setFilenameFormat("/home/smistad/CETUS/Patient1/Patient1_frame#.mhd");
	streamer->setZeroFilling(2);
	streamer->setStartNumber(1);
	streamer->enableLooping();
	streamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
	streamer->update(); // TODO this should not be needed
	streamer->setSleepTime(50);

	ImageSlicer::pointer slicer = ImageSlicer::New();
	slicer->setInputConnection(streamer->getOutputPort());
	slicer->setOrthogonalSlicePlane(PLANE_Y);

	// Control points for spline model
	std::vector<Vector2f> controlPoints = {
	        Vector2f(0, 3), // Apex
	        Vector2f(0.8, 2),
	        Vector2f(1.15, 1),
	        Vector2f(0.8, 0),
	        Vector2f(-0.8, 0),
	        Vector2f(-1.15, 1),
	        Vector2f(-0.8, 2)
	};

	CardinalSplineModel::pointer shapeModel = CardinalSplineModel::New();
	shapeModel->setControlPoints(controlPoints);
	shapeModel->setInitialScaling(0.02, 0.03);
	shapeModel->setInitialRotation(M_PI);
	shapeModel->setGlobalProcessError(0.01f);
	shapeModel->setLocalProcessError(0.001f);
	shapeModel->initializeShapeToImageCenter();
	KalmanFilter::pointer segmentation = KalmanFilter::New();
	StepEdgeModel::pointer appearanceModel = StepEdgeModel::New();
	appearanceModel->setLineLength(0.025);
	appearanceModel->setLineSampleSpacing(0.025/32.0);
	segmentation->setAppearanceModel(appearanceModel);
	segmentation->setShapeModel(shapeModel);
	segmentation->setInputConnection(slicer->getOutputPort());

	MeshRenderer::pointer meshRenderer = MeshRenderer::New();
	meshRenderer->setInputConnection(segmentation->getOutputPort());

	ImageRenderer::pointer imageRenderer = ImageRenderer::New();
	imageRenderer->addInputConnection(slicer->getOutputPort());

	SimpleWindow::pointer window = SimpleWindow::New();
	window->addRenderer(imageRenderer);
	window->addRenderer(meshRenderer);
	window->set2DMode();
	window->start();
}




TEST_CASE("Model based segmentation with ellipse model on 2D femoral nerve block US data", "[fast][ModelBasedSegmentation][visual]") {
	ImageFileStreamer::pointer streamer = ImageFileStreamer::New();
	streamer->setFilenameFormat("/home/smistad/AssistantTestData/FL/US-Acq_01_20150608T102019/Acquisition/US-Acq_01_20150608T102019_Image_Transducer_#.mhd");
	streamer->setStartNumber(26);
	streamer->enableLooping();
	streamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
	streamer->setSleepTime(100);

	EllipseModel::pointer shapeModel = EllipseModel::New();
	shapeModel->setInitialState(Vector2f(27, 10), 5, 4.5);
	KalmanFilter::pointer segmentation = KalmanFilter::New();
	StepEdgeModel::pointer appearanceModel = StepEdgeModel::New();
	appearanceModel->setLineLength(4);
	appearanceModel->setLineSampleSpacing(4.0/16.0);
	segmentation->setAppearanceModel(appearanceModel);
	segmentation->setShapeModel(shapeModel);
	segmentation->setInputConnection(streamer->getOutputPort());

	MeshRenderer::pointer meshRenderer = MeshRenderer::New();
	meshRenderer->setInputConnection(segmentation->getOutputPort());

	ImageRenderer::pointer imageRenderer = ImageRenderer::New();
	imageRenderer->setInputConnection(streamer->getOutputPort());

	SimpleWindow::pointer window = SimpleWindow::New();
	window->addRenderer(imageRenderer);
	window->addRenderer(meshRenderer);
	window->set2DMode();
	window->start();
}
*/
