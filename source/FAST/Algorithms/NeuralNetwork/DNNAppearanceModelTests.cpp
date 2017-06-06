#include "FAST/Testing.hpp"
#include "DNNAppearanceModel.hpp"
#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/Visualization/TriangleRenderer/TriangleRenderer.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"
#include "FAST/Streamers/ImageFileStreamer.hpp"

using namespace fast;

TEST_CASE("Landmark detection", "[fast][DNNAppearanceModel][visual]") {
	ImageFileImporter::pointer importer = ImageFileImporter::New();
	importer->setFilename(Config::getTestDataPath() + "US/FemoralArtery/Right/US-Acq_03_20150608T103739_Image_Transducer_100.mhd");
	DNNAppearanceModel::pointer detector = DNNAppearanceModel::New();
	detector->setInputConnection(importer->getOutputPort());
	detector->loadNetworkAndWeights("/home/smistad/workspace/DNN-AM/femoral_appearance_model_net2/net_deploy.prototxt",
									"/home/smistad/workspace/DNN-AM/femoral_appearance_model_net2/_iter_1000.caffemodel");
	detector->loadObjects("/home/smistad/workspace/DNN-AM/femoral_appearance_model_net2/objects.txt");

	TriangleRenderer::pointer TriangleRenderer = TriangleRenderer::New();
	TriangleRenderer->setInputConnection(detector->getOutputPort());

	ImageRenderer::pointer imageRenderer = ImageRenderer::New();
	imageRenderer->setInputConnection(importer->getOutputPort());

	SimpleWindow::pointer window = SimpleWindow::New();
	window->addRenderer(imageRenderer);
	window->addRenderer(TriangleRenderer);
	window->set2DMode();
	window->start();

}

TEST_CASE("Landmark detection stream", "[fast][DNNAppearanceModel][dynamic][visual]") {
	ImageFileStreamer::pointer importer = ImageFileStreamer::New();
	DNNAppearanceModel::pointer detector = DNNAppearanceModel::New();
	//importer->setFilenameFormat("/home/smistad/AssistantTestData/0/US-Acq_01_20150608T102019/Acquisition/US-Acq_01_20150608T102019_Image_Transducer_#.mhd");
	//importer->setFilenameFormat("/home/smistad/AssistantTestData/1/US-Acq_01_20150608T103428/Acquisition/US-Acq_01_20150608T103428_Image_Transducer_#.mhd");
	//importer->setFilenameFormat("/home/smistad/AssistantTestData/2/US-Acq_01_20150608T104544/Acquisition/US-Acq_01_20150608T104544_Image_Transducer_#.mhd");
	//importer->setFilenameFormat("/home/smistad/AssistantTestData/2/US-Acq_02_20150608T104623/Acquisition/US-Acq_02_20150608T104623_Image_Transducer_#.mhd");
	//importer->setFilenameFormat("/home/smistad/AssistantTestData/4/US-Acq_01_20150608T112522/Acquisition/US-Acq_01_20150608T112522_Image_Transducer_#.mhd");
	importer->setFilenameFormat("/home/smistad/AssistantTestData/1/2016-08-09-right/US-2D_#.mhd");
	//importer->setFilenameFormat("/home/smistad/AssistantTestData/1/2016-08-09-left/US-2D_#.mhd");
	//detector->setMirrorImage(true); // Set this to true for left side images
	importer->setSleepTime(75);
	detector->setInputConnection(importer->getOutputPort());
    detector->loadNetworkAndWeights("/home/smistad/workspace/DNN-AM/femoral_appearance_model_net2/net_deploy.prototxt",
                        "/home/smistad/workspace/DNN-AM/femoral_appearance_model_net2/_iter_1000.caffemodel");
	detector->loadObjects("/home/smistad/workspace/DNN-AM/femoral_appearance_model_net2/objects.txt");

	TriangleRenderer::pointer TriangleRenderer = TriangleRenderer::New();
	TriangleRenderer->setInputConnection(detector->getOutputPort());
	TriangleRenderer->setColor(0, Color::Red());
	TriangleRenderer->setColor(1, Color::Yellow());
	TriangleRenderer->setLineSize(4);

	ImageRenderer::pointer imageRenderer = ImageRenderer::New();
	imageRenderer->setInputConnection(importer->getOutputPort());

	SimpleWindow::pointer window = SimpleWindow::New();
	window->addRenderer(imageRenderer);
	window->addRenderer(TriangleRenderer);
	window->set2DMode();
	window->enableFullscreen();
	window->setWidth(1024);
	window->setHeight(1024);
	window->start();

}
