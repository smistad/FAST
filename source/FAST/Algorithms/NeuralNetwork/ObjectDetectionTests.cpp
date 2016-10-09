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
	ObjectDetection::pointer detector = ObjectDetection::New();
	//importer->setFilenameFormat("/home/smistad/AssistantTestData/0/US-Acq_01_20150608T102019/Acquisition/US-Acq_01_20150608T102019_Image_Transducer_#.mhd");
	//importer->setFilenameFormat("/home/smistad/AssistantTestData/1/US-Acq_01_20150608T103428/Acquisition/US-Acq_01_20150608T103428_Image_Transducer_#.mhd");
	importer->setFilenameFormat("/home/smistad/AssistantTestData/1/US-Acq_03_20150608T103739/Acquisition/US-Acq_03_20150608T103739_Image_Transducer_#.mhd");
	//importer->setFilenameFormat("/home/smistad/AssistantTestData/2/US-Acq_01_20150608T104544/Acquisition/US-Acq_01_20150608T104544_Image_Transducer_#.mhd");
	//importer->setFilenameFormat("/home/smistad/AssistantTestData/2/US-Acq_02_20150608T104623/Acquisition/US-Acq_02_20150608T104623_Image_Transducer_#.mhd");
	//importer->setFilenameFormat("/home/smistad/AssistantTestData/2/US-Acq_03_20150608T104805/Acquisition/US-Acq_03_20150608T104805_Image_Transducer_#.mhd");
	//importer->setFilenameFormat("/home/smistad/AssistantTestData/2/US-Acq_04_20150608T104910/Acquisition/US-Acq_04_20150608T104910_Image_Transducer_#.mhd");
	//importer->setFilenameFormat("/home/smistad/AssistantTestData/4/US-Acq_01_20150608T112522/Acquisition/US-Acq_01_20150608T112522_Image_Transducer_#.mhd");
	//importer->setFilenameFormat("/home/smistad/AssistantTestData/9/0/US-2D_#.mhd");
	//importer->setFilenameFormat("/home/smistad/AssistantTestData/9/2/US-2D_#.mhd");
	//importer->setFilenameFormat("/home/smistad/AssistantTestData/11/2/US-2D_#.mhd");
	//importer->setFilenameFormat("/media/smistad/New Volume/Carotis/Daniel/2016-06-27-143748-carotis_daniel/US-2D_#.mhd");
	//importer->setFilenameFormat("/home/smistad/AssistantTestData/1/2016-08-09-right/US-2D_#.mhd");
	//importer->setFilenameFormat("/home/smistad/AssistantTestData/1/2016-08-09-left/US-2D_#.mhd");
	//detector->setMirrorImage(true); // Set this to true for left side images
	importer->setSleepTime(75);
	detector->setInputConnection(importer->getOutputPort());
	detector->loadNetworkAndWeights("/home/smistad/workspace/detect_femoral_artery/net_deploy.prototxt",
						"/home/smistad/workspace/detect_femoral_artery/models/_iter_6000.caffemodel");

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

}
