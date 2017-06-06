#include "FAST/Testing.hpp"
#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/Visualization/VertexRenderer/VertexRenderer.hpp"
#include "FAST/Visualization/TriangleRenderer/TriangleRenderer.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"
#include "FAST/Streamers/ImageFileStreamer.hpp"
#include "ObjectDetection.hpp"

using namespace fast;

/*
TEST_CASE("Object detection stream", "[fast][ObjectDetection][dynamic][visual]") {
	ImageFileStreamer::pointer importer = ImageFileStreamer::New();
    importer->setFilenameFormat("/media/extra/EyeGuide/Axille/0/2016-10-07-135111/US-2D_#.mhd");
    importer->setFilenameFormats({
         "/home/smistad/data/eyeguide/axillary_nerve_block/5/2016Dec30_082229/#.png",
         "/home/smistad/data/eyeguide/axillary_nerve_block/5/2016Dec30_082309/#.png",
         "/home/smistad/data/eyeguide/axillary_nerve_block/5/2016Dec30_082046/#.png",
         "/home/smistad/data/eyeguide/axillary_nerve_block/5/2016Dec30_082309/#.png",
         "/home/smistad/data/eyeguide/axillary_nerve_block/11/2017Jan18_094316/#.png",
		 "/home/smistad/data/eyeguide/axillary_nerve_block/11/2017Jan18_094418/#.png",
         "/home/smistad/data/eyeguide/axillary_nerve_block/6/2016Dec30_084715/#.png",
		 "/home/smistad/data/eyeguide/axillary_nerve_block/6/2016Dec30_084746/#.png",
     });
    importer->setStartNumber(1);
	importer->setSleepTime(50);

	ObjectDetection::pointer detector = ObjectDetection::New();
	//detector->setMirrorImage(true); // Set this to true for left side images
    detector->enableRuntimeMeasurements();
	detector->setInputConnection(importer->getOutputPort());
    detector->load("/home/smistad/workspace/eyeguide_keras/models/network_graph.pb");
	detector->setInputSize(256, 256);
    detector->setScaleFactor(1.0f/255.0f);

	TriangleRenderer::pointer TriangleRenderer = TriangleRenderer::New();
	TriangleRenderer->setInputConnection(detector->getOutputPort());
	TriangleRenderer->setColor(1, Color::Red());
	TriangleRenderer->setColor(2, Color(0.2, 0.2, 1.0));
	TriangleRenderer->setColor(3, Color::Green());
	TriangleRenderer->setColor(4, Color::Purple());
	TriangleRenderer->setColor(5, Color::Cyan());
	TriangleRenderer->setLineSize(4);

	ImageRenderer::pointer imageRenderer = ImageRenderer::New();
	imageRenderer->setInputConnection(importer->getOutputPort());

	SimpleWindow::pointer window = SimpleWindow::New();
	window->addRenderer(imageRenderer);
	window->addRenderer(TriangleRenderer);
	window->set2DMode();
	window->getView()->setBackgroundColor(Color::Black());
	//window->enableFullscreen();
	window->setWidth(1920);
	window->setHeight(1080);
	window->start();

	detector->getAllRuntimes()->printAll();

}
 */
