#include "FAST/Testing.hpp"
#include "LandmarkDetection.hpp"
#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/Visualization/MeshRenderer/MeshRenderer.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"
#include "FAST/Streamers/ImageFileStreamer.hpp"

using namespace fast;

TEST_CASE("Landmark detection", "[fast][LandmarkDetection]") {
	ImageFileImporter::pointer importer = ImageFileImporter::New();
	importer->setFilename("/home/smistad/Dropbox/Notebooks/Model based segmentation/datasets/2/3/US-2D_250.png");
	LandmarkDetection::pointer detector = LandmarkDetection::New();
	detector->setInputConnection(importer->getOutputPort());
	detector->loadModel("/home/smistad/Dropbox/Notebooks/Model based segmentation/training/net_deploy.prototxt", "/home/smistad/snapshots/_iter_1000.caffemodel", "/home/smistad/workspace/DNN-AM/objects.txt");

	MeshRenderer::pointer meshRenderer = MeshRenderer::New();
	meshRenderer->setInputConnection(detector->getOutputPort());

	ImageRenderer::pointer imageRenderer = ImageRenderer::New();
	imageRenderer->setInputConnection(importer->getOutputPort());

	SimpleWindow::pointer window = SimpleWindow::New();
	window->addRenderer(imageRenderer);
	window->addRenderer(meshRenderer);
	window->set2DMode();
	window->start();

}

TEST_CASE("Landmark detection stream", "[fast][LandmarkDetection][dynamic]") {
	ImageFileStreamer::pointer importer = ImageFileStreamer::New();
	LandmarkDetection::pointer detector = LandmarkDetection::New();
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
	detector->loadModel("/home/smistad/workspace/DNN-AM/femoral_appearance_model_net2/net_deploy.prototxt", "/home/smistad/workspace/DNN-AM/femoral_appearance_model_net2/_iter_1000.caffemodel", "/home/smistad/workspace/DNN-AM/femoral_appearance_model_net2/objects.txt");

	MeshRenderer::pointer meshRenderer = MeshRenderer::New();
	meshRenderer->setInputConnection(detector->getOutputPort());
	meshRenderer->setColor(0, Color::Red());
	meshRenderer->setColor(1, Color::Yellow());
	meshRenderer->setLineSize(4);

	ImageRenderer::pointer imageRenderer = ImageRenderer::New();
	imageRenderer->setInputConnection(importer->getOutputPort());

	SimpleWindow::pointer window = SimpleWindow::New();
	window->addRenderer(imageRenderer);
	window->addRenderer(meshRenderer);
	window->set2DMode();
	window->enableFullscreen();
	window->setWidth(1024);
	window->setHeight(1024);
	window->start();

}
