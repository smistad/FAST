#include "FAST/Testing.hpp"
#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/Visualization/PointRenderer/PointRenderer.hpp"
#include "FAST/Visualization/MeshRenderer/MeshRenderer.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"
#include "FAST/Streamers/ImageFileStreamer.hpp"
#include "ObjectDetection.hpp"

using namespace fast;


TEST_CASE("Object detection stream", "[fast][ObjectDetection][dynamic][visual]") {
	ImageFileStreamer::pointer importer = ImageFileStreamer::New();
    //importer->setFilenameFormat("/media/smistad/New Volume/EyeGuide/Axille/0/2016-10-07-135111/US-2D_#.mhd");
    importer->setFilenameFormat("/media/smistad/New Volume/EyeGuide/Axille/1/2016-10-07-135630/US-2D_#.mhd");
	importer->setSleepTime(75);

	ObjectDetection::pointer detector = ObjectDetection::New();
	//detector->setMirrorImage(true); // Set this to true for left side images
    detector->enableRuntimeMeasurements();
	detector->setInputConnection(importer->getOutputPort());
    detector->load("/home/smistad/workspace/eyeguide_keras/models/network_graph.pb");
	detector->setInputParameters("input_image", 256, 256);

	MeshRenderer::pointer meshRenderer = MeshRenderer::New();
	meshRenderer->setInputConnection(detector->getOutputPort());
	meshRenderer->setDefaultColor(Color::Red());
	meshRenderer->setLineSize(4);

	ImageRenderer::pointer imageRenderer = ImageRenderer::New();
	imageRenderer->setInputConnection(importer->getOutputPort());

	SimpleWindow::pointer window = SimpleWindow::New();
	window->addRenderer(imageRenderer);
	window->addRenderer(meshRenderer);
	window->set2DMode();
	//window->enableFullscreen();
	window->setWidth(1920);
	window->setHeight(1080);
	window->start();

	detector->getAllRuntimes()->printAll();

}
