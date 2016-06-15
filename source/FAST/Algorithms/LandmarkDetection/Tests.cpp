#include "FAST/Testing.hpp"
#include "LandmarkDetection.hpp"
#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/Visualization/PointRenderer/PointRenderer.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"
#include "FAST/Streamers/ImageFileStreamer.hpp"

using namespace fast;

TEST_CASE("Landmark detection", "[fast][LandmarkDetection]") {
	ImageFileImporter::pointer importer = ImageFileImporter::New();
	importer->setFilename("/home/smistad/Dropbox/Notebooks/Model based segmentation/datasets/2/3/US-2D_250.png");
	LandmarkDetection::pointer detector = LandmarkDetection::New();
	detector->setInputConnection(importer->getOutputPort());
	detector->loadModel("/home/smistad/Dropbox/Notebooks/Model based segmentation/training/net_deploy.prototxt", "/home/smistad/snapshots/_iter_1000.caffemodel");

	PointRenderer::pointer pointRenderer = PointRenderer::New();
	pointRenderer->setInputConnection(detector->getOutputPort());

	ImageRenderer::pointer imageRenderer = ImageRenderer::New();
	imageRenderer->setInputConnection(importer->getOutputPort());

	SimpleWindow::pointer window = SimpleWindow::New();
	window->addRenderer(imageRenderer);
	window->addRenderer(pointRenderer);
	window->set2DMode();
	window->start();

}

TEST_CASE("Landmark detection stream", "[fast][LandmarkDetection][dynamic]") {
	ImageFileStreamer::pointer importer = ImageFileStreamer::New();
	//importer->setFilenameFormat("/home/smistad/AssistantTestData/0/US-Acq_01_20150608T102019/Acquisition/US-Acq_01_20150608T102019_Image_Transducer_#.mhd");
	//importer->setFilenameFormat("/home/smistad/AssistantTestData/2/US-Acq_01_20150608T104544/Acquisition/US-Acq_01_20150608T104544_Image_Transducer_#.mhd");
	importer->setFilenameFormat("/home/smistad/AssistantTestData/4/US-Acq_01_20150608T112522/Acquisition/US-Acq_01_20150608T112522_Image_Transducer_#.mhd");
	importer->setSleepTime(100);
	LandmarkDetection::pointer detector = LandmarkDetection::New();
	detector->setInputConnection(importer->getOutputPort());
	detector->loadModel("/home/smistad/Dropbox/Notebooks/Model based segmentation/training/net_deploy.prototxt", "/home/smistad/snapshots/_iter_1000.caffemodel");

	PointRenderer::pointer pointRenderer = PointRenderer::New();
	pointRenderer->setInputConnection(detector->getOutputPort());

	ImageRenderer::pointer imageRenderer = ImageRenderer::New();
	imageRenderer->setInputConnection(importer->getOutputPort());

	SimpleWindow::pointer window = SimpleWindow::New();
	window->addRenderer(imageRenderer);
	window->addRenderer(pointRenderer);
	window->set2DMode();
	window->setWidth(1024);
	window->setHeight(1024);
	window->start();

}
