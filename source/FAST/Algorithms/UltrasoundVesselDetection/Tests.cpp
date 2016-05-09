#include "FAST/Testing.hpp"
#include "UltrasoundVesselDetection.hpp"
#include "FAST/Streamers/ImageFileStreamer.hpp"
#include "FAST/Data/Image.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SegmentationRenderer/SegmentationRenderer.hpp"

using namespace fast;

TEST_CASE("UltrasoundVesselDetection", "[fast][UltrasoundVesselDetection][visual]") {
	ImageFileStreamer::pointer streamer = ImageFileStreamer::New();
	streamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
	//streamer->setFilenameFormat("/home/smistad/AssistantTestData/ES/US-Acq_03_20150608T103739/Acquisition/US-Acq_03_20150608T103739_Image_Transducer_#.mhd");
	streamer->setFilenameFormat("/home/smistad/AssistantTestData/ES/US-Acq_04_20150608T103837/Acquisition/US-Acq_04_20150608T103837_Image_Transducer_#.mhd");
	//streamer->setFilenameFormat("/home/smistad/ES/US-Acq_07_20150608T104148/Acquisition/US-Acq_07_20150608T104148_Image_Transducer_#.mhd");
	//streamer->setFilenameFormat("/home/smistad/AssistantTestData/ES/US-Acq_08_20150608T104238/Acquisition/US-Acq_08_20150608T104238_Image_Transducer_#.mhd");
	//streamer->setFilenameFormat("/home/smistad/AssistantTestData/FL/US-Acq_03_20150608T102144/Acquisition/US-Acq_03_20150608T102144_Image_Transducer_#.mhd");
	//streamer->setFilenameFormat("/home/smistad/AssistantTestData/FL/US-Acq_04_20150608T102242/Acquisition/US-Acq_04_20150608T102242_Image_Transducer_#.mhd");
	//streamer->setFilenameFormat("/home/smistad/AssistantTestData/DHI/US-Acq_03_20150608T104805/Acquisition/US-Acq_03_20150608T104805_Image_Transducer_#.mhd");
	//streamer->setFilenameFormat("/home/smistad/AssistantTestData/DHI/US-Acq_04_20150608T104910/Acquisition/US-Acq_04_20150608T104910_Image_Transducer_#.mhd");
	//streamer->setFilenameFormat("/home/smistad/AssistantTestData/JBB/US-Acq_03_20150608T113646/Acquisition/US-Acq_03_20150608T113646_Image_Transducer_#.mhd");
	//streamer->setFilenameFormat("/home/smistad/AssistantTestData/JBB/US-Acq_07_20150608T114115/Acquisition/US-Acq_07_20150608T114115_Image_Transducer_#.mhd");
	//streamer->setFilenameFormat("/home/smistad/AssistantTestData/IR/US-Acq_03_20150608T112610/Acquisition/US-Acq_03_20150608T112610_Image_Transducer_#.mhd");
	//streamer->setFilenameFormat("/home/smistad/AssistantTestData/IR/US-Acq_04_20150608T112656/Acquisition/US-Acq_04_20150608T112656_Image_Transducer_#.mhd");
	//streamer->setFilenameFormat("/home/smistad/AssistantTestData/LL/US-Acq_03_20150608T111610/Acquisition/US-Acq_03_20150608T111610_Image_Transducer_#.mhd");
	//streamer->setFilenameFormat("/home/smistad/AssistantTestData/LL/US-Acq_04_20150608T111701/Acquisition/US-Acq_04_20150608T111701_Image_Transducer_#.mhd");
	streamer->update();

	UltrasoundVesselDetection::pointer detector = UltrasoundVesselDetection::New();
	detector->setInputConnection(streamer->getOutputPort());
	detector->enableRuntimeMeasurements();

	SegmentationRenderer::pointer segmentationRenderer = SegmentationRenderer::New();
	segmentationRenderer->addInputConnection(detector->getOutputImagePort());
	segmentationRenderer->setFillArea(false);

	ImageRenderer::pointer imageRenderer = ImageRenderer::New();
	imageRenderer->addInputConnection(streamer->getOutputPort());

	SimpleWindow::pointer window = SimpleWindow::New();
	window->addRenderer(imageRenderer);
	window->addRenderer(segmentationRenderer);
	window->set2DMode();
	window->setWidth(1024);
	window->setHeight(1024);
	window->start();
	detector->getRuntime("ellipse fitting")->print();
	detector->getRuntime("candidate selection")->print();
	detector->getRuntime("classifier")->print();
	detector->getRuntime("segmentation")->print();

}
