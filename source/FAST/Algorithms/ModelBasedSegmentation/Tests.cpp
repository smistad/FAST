#include "FAST/Testing.hpp"
#include "FAST/Streamers/ImageFileStreamer.hpp"
#include "KalmanFilter.hpp"
#include "FAST/Visualization/TriangleRenderer/TriangleRenderer.hpp"
#include "FAST/Visualization/SliceRenderer/SliceRenderer.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"
#include "FAST/Algorithms/ImageSlicer/ImageSlicer.hpp"
#include "AppearanceModels/StepEdge/StepEdgeModel.hpp"
#include "AppearanceModels/RidgeEdge/RidgeEdgeModel.hpp"
#include "ShapeModels/MeanValueCoordinates/MeanValueCoordinatesModel.hpp"
#include "ShapeModels/Ellipse/EllipseModel.hpp"
#include "ShapeModels/CardinalSpline/CardinalSplineModel.hpp"
#include "FAST/Algorithms/MeshToSegmentation/MeshToSegmentation.hpp"
#include "FAST/Algorithms/SurfaceExtraction/SurfaceExtraction.hpp"

using namespace fast;

/*
TEST_CASE("Model based segmentation with mean value coordinates on 3D cardiac US data", "[fast][ModelBasedSegmentation][cardiac][3d][visual]") {
	ImageFileStreamer::pointer streamer = ImageFileStreamer::New();
	streamer->setFilenameFormat("/media/extra/CETUS/Patient1/Patient1_frame#.mhd");
	streamer->setZeroFilling(2);
	streamer->setStartNumber(1);
	streamer->enableLooping();
	streamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
	//streamer->setSleepTime(2000);

	MeanValueCoordinatesModel::pointer shapeModel = MeanValueCoordinatesModel::New();
	shapeModel->loadMeshes(Config::getTestDataPath() + "cetus_model_mesh_small.vtk",
			Config::getTestDataPath() + "cetus_control_mesh.vtk");
	shapeModel->initializeShapeToImageCenter();
	KalmanFilter::pointer segmentation = KalmanFilter::New();
	RidgeEdgeModel::pointer appearanceModel = RidgeEdgeModel::New();
	appearanceModel->setLineLength(0.025);
	appearanceModel->setLineSampleSpacing(0.025/48.0);
	appearanceModel->setEdgeType(RidgeEdgeModel::EDGE_TYPE_BLACK_INSIDE_WHITE_OUTSIDE);
	appearanceModel->setIntensityDifferenceThreshold(20);
    appearanceModel->setMinimumRidgeSize(3.0f/1000);
	segmentation->setAppearanceModel(appearanceModel);
	segmentation->setShapeModel(shapeModel);
	segmentation->setInputConnection(streamer->getOutputPort());


	MeshToSegmentation::pointer meshToSeg = MeshToSegmentation::New();
	meshToSeg->setInputConnection(0, segmentation->getOutputPort());
	meshToSeg->setInputConnection(1, streamer->getOutputPort());

	SurfaceExtraction::pointer extraction = SurfaceExtraction::New();
	extraction->setInputConnection(meshToSeg->getOutputPort());

	TriangleRenderer::pointer TriangleRenderer = TriangleRenderer::New();
	TriangleRenderer->setInputConnection(segmentation->getOutputPort());
	TriangleRenderer->setDefaultOpacity(0.5);

	SliceRenderer::pointer sliceRenderer = SliceRenderer::New();
	sliceRenderer->setInputConnection(streamer->getOutputPort());
	sliceRenderer->setSlicePlane(PLANE_Y);
	SliceRenderer::pointer sliceRenderer2 = SliceRenderer::New();
	sliceRenderer2->setInputConnection(streamer->getOutputPort());
	sliceRenderer2->setSlicePlane(PLANE_X);

	SimpleWindow::pointer window = SimpleWindow::New();
	window->addRenderer(sliceRenderer);
	window->addRenderer(sliceRenderer2);
	window->addRenderer(TriangleRenderer);
	window->start();
}

TEST_CASE("Model based segmentation with spline model on 2D pediatric cardiac US data", "[fast][ModelBasedSegmentation][2d][cardiac][pediatric][visual]") {
	ImageFileStreamer::pointer streamer = ImageFileStreamer::New();
	streamer->setFilenameFormat("/home/smistad/Cardiac_2D/test3/labelImage#.mhd");
	streamer->setZeroFilling(2);
	streamer->enableLooping();
	streamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
	//streamer->setSleepTime(500);

	// Control points for spline model
	std::vector<Vector2f> controlPoints = {
//			Vector2f(2.822184520427274634e+01, 2.204189742133431196e+01),
//			Vector2f(2.805852557576601924e+01, 3.001778981203989716e+01),
//			Vector2f(2.981067073476717511e+01, 4.219496931887970703e+01),
//			Vector2f(4.537380221088420029e+01, 4.078267995844019822e+01),
//			Vector2f(4.447638939087526921e+01, 2.948818130187507691e+01),
//			Vector2f(3.884196391885149779e+01, 2.151228891116949882e+01)

			Vector2f(3.270129660437638819e+01, 1.680781070226831275e+01),
			Vector2f(3.016050498811447511e+01, 1.302153299960351518e+01),
			Vector2f(2.301053848817431202e+01, 2.308916615809692985e+01),
			Vector2f(2.181487184522752543e+01, 3.815631858720144720e+01),
			Vector2f(3.344448267586006551e+01, 3.616354084895680643e+01),
			Vector2f(3.404642157769151112e+01, 2.871465768827996001e+01),
			Vector2f(3.344448267586006551e+01, 2.229205506279907567e+01),
	};

	CardinalSplineModel::pointer shapeModel = CardinalSplineModel::New();
	shapeModel->setControlPoints(controlPoints);
	shapeModel->setGlobalProcessError(0.000001f);
	shapeModel->setLocalProcessError(0.0000001f);
	shapeModel->setResolution(12);
	shapeModel->setScalingLimit(0.5);
    //shapeModel->setInitialScaling(1.5, 1.5);
	KalmanFilter::pointer segmentation = KalmanFilter::New();
	RidgeEdgeModel::pointer appearanceModel = RidgeEdgeModel::New();
	appearanceModel->setLineLength(8);
	appearanceModel->setLineSampleSpacing(8.0/48.0);
	appearanceModel->setIntensityDifferenceThreshold(30);
	appearanceModel->setMinimumDepth(10);
    appearanceModel->setEdgeType(RidgeEdgeModel::EDGE_TYPE_BLACK_INSIDE_WHITE_OUTSIDE);
	segmentation->setStartIterations(10);
	segmentation->setIterations(10);
	segmentation->setAppearanceModel(appearanceModel);
	segmentation->setShapeModel(shapeModel);
	segmentation->setInputConnection(streamer->getOutputPort());

    MeshToSegmentation::pointer meshToSeg = MeshToSegmentation::New();
    meshToSeg->setInputConnection(0, segmentation->getOutputPort());
	meshToSeg->setInputConnection(1, streamer->getOutputPort());

	TriangleRenderer::pointer TriangleRenderer = TriangleRenderer::New();
	TriangleRenderer->addInputConnection(segmentation->getOutputPort());
	TriangleRenderer->addInputConnection(segmentation->getDisplacementsOutputPort(), Color::Red(), 1.0);

	ImageRenderer::pointer imageRenderer = ImageRenderer::New();
	imageRenderer->addInputConnection(streamer->getOutputPort());

	SimpleWindow::pointer window = SimpleWindow::New();
	window->addRenderer(imageRenderer);
	window->addRenderer(TriangleRenderer);
	window->set2DMode();
    window->setWidth(1920);
	window->setHeight(1080);
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
	RidgeEdgeModel::pointer appearanceModel = RidgeEdgeModel::New();
	appearanceModel->setLineLength(8);
	appearanceModel->setLineSampleSpacing(8/32.0);
	appearanceModel->setIntensityDifferenceThreshold(10);
	appearanceModel->setMinimumDepth(15);
    appearanceModel->setEdgeType(RidgeEdgeModel::EDGE_TYPE_BLACK_INSIDE_WHITE_OUTSIDE);
	segmentation->setStartIterations(10);
	segmentation->setIterations(10);
	segmentation->setAppearanceModel(appearanceModel);
	segmentation->setShapeModel(shapeModel);
	segmentation->setInputConnection(streamer->getOutputPort());

	TriangleRenderer::pointer TriangleRenderer = TriangleRenderer::New();
	TriangleRenderer->addInputConnection(segmentation->getOutputPort());
	TriangleRenderer->addInputConnection(segmentation->getDisplacementsOutputPort(), Color::Red(), 1.0);

	ImageRenderer::pointer imageRenderer = ImageRenderer::New();
	imageRenderer->addInputConnection(streamer->getOutputPort());

	SimpleWindow::pointer window = SimpleWindow::New();
	window->addRenderer(imageRenderer);
	window->addRenderer(TriangleRenderer);
	window->set2DMode();
	window->start();
}



TEST_CASE("Model based segmentation with ellipse model on 2D femoral nerve block US data", "[fast][ModelBasedSegmentation][femoral][2d][visual]") {
	ImageFileStreamer::pointer streamer = ImageFileStreamer::New();
	streamer->setFilenameFormat("/home/smistad/AssistantTestData/0/US-Acq_01_20150608T102019/Acquisition/US-Acq_01_20150608T102019_Image_Transducer_#.mhd");
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

	TriangleRenderer::pointer TriangleRenderer = TriangleRenderer::New();
	TriangleRenderer->setInputConnection(segmentation->getOutputPort());

	ImageRenderer::pointer imageRenderer = ImageRenderer::New();
	imageRenderer->setInputConnection(streamer->getOutputPort());

	SimpleWindow::pointer window = SimpleWindow::New();
	window->addRenderer(imageRenderer);
	window->addRenderer(TriangleRenderer);
	window->set2DMode();
	window->start();
}
 */


TEST_CASE("Model based segmentation with spline model on 2D cardiac US data", "[fast][ModelBasedSegmentation][2d][cardiac][visual]") {
	ImageFileStreamer::pointer streamer = ImageFileStreamer::New();
	//streamer->setFilenameFormat(Config::getTestDataPath()+"US/Heart/ApicalTwoChamber/US-2D_#.mhd");
	//streamer->setFilenameFormat(Config::getTestDataPath()+"US/Heart/ApicalLongAxis/US-2D_#.mhd");
    streamer->setFilenameFormat(Config::getTestDataPath()+"US/Heart/ApicalFourChamber/US-2D_#.mhd");
	//streamer->setFilenameFormat("/home/smistad/data/ultrasound_smistad_heart/1234/H1ADBNGK/US-2D_#.mhd");
	//streamer->setFilenameFormat("/home/smistad/data/ultrasound_smistad_heart/1234/H1ADCKOO/US-2D_#.mhd");
	//streamer->setFilenameFormat("/home/smistad/data/ultrasound_smistad_heart/1234/H1ADCL8Q/US-2D_#.mhd");

	streamer->enableLooping();
	streamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
	streamer->setSleepTime(50);

	// Control points for spline model
	std::vector<Vector2f> controlPoints = {
			Vector2f(89,22), // Apex
			Vector2f(100,31),
			Vector2f(108,50),
			Vector2f(114,63),
			Vector2f(117,80),
			Vector2f(113,95),
			Vector2f(96,102),
			Vector2f(75,99),
			Vector2f(73,81),
			Vector2f(75,61),
			Vector2f(72,43),
			Vector2f(80,31),
	};

	CardinalSplineModel::pointer shapeModel = CardinalSplineModel::New();
	shapeModel->setControlPoints(controlPoints);
	// Increasing these will put more weight on the measurements instead of the model, and vica versa
	shapeModel->setGlobalProcessError(0.000001f);
	shapeModel->setLocalProcessError(0.001f);
	shapeModel->initializeShapeToImageCenter();
	KalmanFilter::pointer segmentation = KalmanFilter::New();
	/*
	RidgeEdgeModel::pointer appearanceModel = RidgeEdgeModel::New();
	appearanceModel->setEdgeType(RidgeEdgeModel::EDGE_TYPE_BLACK_INSIDE_WHITE_OUTSIDE);
	appearanceModel->setIntensityDifferenceThreshold(20);
	appearanceModel->setMinimumRidgeSize(3.0f);
	 */
	StepEdgeModel::pointer appearanceModel = StepEdgeModel::New();
	appearanceModel->setEdgeType(StepEdgeModel::EDGE_TYPE_BLACK_INSIDE_WHITE_OUTSIDE);
	appearanceModel->setIntensityDifferenceThreshold(20);

	appearanceModel->setLineLength(30.0);
	appearanceModel->setLineSampleSpacing(30.0/48.0);
	segmentation->setAppearanceModel(appearanceModel);
	segmentation->setShapeModel(shapeModel);
	segmentation->setInputConnection(streamer->getOutputPort());
    segmentation->setIterations(10);
	segmentation->setStartIterations(5);

	TriangleRenderer::pointer TriangleRenderer = TriangleRenderer::New();
	TriangleRenderer->addInputConnection(segmentation->getOutputPort());
	TriangleRenderer->addInputConnection(segmentation->getDisplacementsOutputPort(), Color::Red(), 1.0);

	ImageRenderer::pointer imageRenderer = ImageRenderer::New();
	imageRenderer->addInputConnection(streamer->getOutputPort());

	SimpleWindow::pointer window = SimpleWindow::New();
	window->getView()->setBackgroundColor(Color::Black());
	window->addRenderer(imageRenderer);
	window->addRenderer(TriangleRenderer);
	window->setSize(1024, 1024);
	window->set2DMode();
	window->setTimeout(1000);
	window->start();
}
