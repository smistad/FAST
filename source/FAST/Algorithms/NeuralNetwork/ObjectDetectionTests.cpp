#include "FAST/Testing.hpp"
#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/Visualization/PointRenderer/PointRenderer.hpp"
#include "FAST/Visualization/MeshRenderer/MeshRenderer.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"
#include "FAST/Streamers/ImageFileStreamer.hpp"
#include "ObjectDetection.hpp"

using namespace fast;

/*
TEST_CASE("Object detection stream", "[fast][ObjectDetection][dynamic][visual]") {
	ImageFileStreamer::pointer importer = ImageFileStreamer::New();
    //importer->setFilenameFormat("/media/smistad/New Volume/EyeGuide/Axille/0/2016-10-07-135111/US-2D_#.mhd");
    importer->setFilenameFormats({
         "/home/smistad/data/eyeguide/axillary_nerve_block/11/2017Jan18_094316/#.png",
		 "/home/smistad/data/eyeguide/axillary_nerve_block/11/2017Jan18_094418/#.png",
         "/home/smistad/data/eyeguide/axillary_nerve_block/6/2016Dec30_084715/#.png",
		 "/home/smistad/data/eyeguide/axillary_nerve_block/6/2016Dec30_084746/#.png",
         "/home/smistad/data/eyeguide/axillary_nerve_block/5/2016Dec30_082309/#.png",
		 "/home/smistad/data/eyeguide/axillary_nerve_block/5/2016Dec30_082046/#.png",
		 "/home/smistad/data/eyeguide/axillary_nerve_block/5/2016Dec30_082229/#.png",
		 "/home/smistad/data/eyeguide/axillary_nerve_block/5/2016Dec30_082309/#.png",
     });
    importer->setStartNumber(1);
	importer->setSleepTime(75);

	ObjectDetection::pointer detector = ObjectDetection::New();
	//detector->setMirrorImage(true); // Set this to true for left side images
    detector->enableRuntimeMeasurements();
	detector->setInputConnection(importer->getOutputPort());
    detector->load("/home/smistad/workspace/eyeguide_keras/models/network_graph.pb");
	detector->setInputSize(256, 256);
    detector->setScaleFactor(1.0f/255.0f);

	MeshRenderer::pointer meshRenderer = MeshRenderer::New();
	meshRenderer->setInputConnection(detector->getOutputPort());
	meshRenderer->setColor(1, Color::Red());
	meshRenderer->setColor(2, Color::Blue());
	meshRenderer->setColor(3, Color::Green());
	meshRenderer->setColor(4, Color::Purple());
	meshRenderer->setColor(5, Color::Cyan());
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
 */
