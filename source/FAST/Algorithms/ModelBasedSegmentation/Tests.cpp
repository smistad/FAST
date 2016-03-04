#include "FAST/Testing.hpp"
#include "FAST/Streamers/ImageFileStreamer.hpp"
#include "KalmanFilter.hpp"
#include "FAST/Visualization/MeshRenderer/MeshRenderer.hpp"
#include "FAST/Visualization/SliceRenderer/SliceRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"
#include "AppearanceModels/StepEdge/StepEdgeModel.hpp"
#include "ShapeModels/MeanValueCoordinates/MeanValueCoordinatesModel.hpp"

using namespace fast;

TEST_CASE("", "[fast][ModelBasedSegmentation]") {
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
	meshRenderer->setDefaultOpacity(0.3);

	SliceRenderer::pointer sliceRenderer = SliceRenderer::New();
	sliceRenderer->setInputConnection(streamer->getOutputPort());
	sliceRenderer->setSlicePlane(PLANE_Y);

	SimpleWindow::pointer window = SimpleWindow::New();
	window->addRenderer(meshRenderer);
	window->addRenderer(sliceRenderer);
	window->start();
}
