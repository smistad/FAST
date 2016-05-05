#include "FAST/Testing.hpp"
#include "UltrasoundVesselDetection.hpp"
#include "FAST/Streamers/ImageFileStreamer.hpp"
#include "FAST/Data/Image.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SegmentationRenderer/SegmentationRenderer.hpp"

using namespace fast;

TEST_CASE("UltrasoundVesselDetection", "[fast][UltrasoundVesselDetection]") {
	ImageFileStreamer::pointer streamer = ImageFileStreamer::New();
	streamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
	//streamer->setFilenameFormat("/home/smistad/AssistantTestData/ES/US-Acq_03_20150608T103739/Acquisition/US-Acq_03_20150608T103739_Image_Transducer_#.mhd");
	streamer->setFilenameFormat("/home/smistad/AssistantTestData/ES/US-Acq_04_20150608T103837/Acquisition/US-Acq_04_20150608T103837_Image_Transducer_#.mhd");
	//streamer->setFilenameFormat("/home/smistad/ES/US-Acq_07_20150608T104148/Acquisition/US-Acq_07_20150608T104148_Image_Transducer_#.mhd");
	//streamer->setFilenameFormat("/home/smistad/AssistantTestData/ES/US-Acq_08_20150608T104238/Acquisition/US-Acq_08_20150608T104238_Image_Transducer_#.mhd");
	//streamer->setStepSize(50);
	streamer->update();

	UltrasoundVesselDetection::pointer detector = UltrasoundVesselDetection::New();
	detector->setInputConnection(streamer->getOutputPort());

	SegmentationRenderer::pointer segmentationRenderer = SegmentationRenderer::New();
	segmentationRenderer->addInputConnection(detector->getOutputImagePort());

	ImageRenderer::pointer imageRenderer = ImageRenderer::New();
	imageRenderer->addInputConnection(streamer->getOutputPort());

	SimpleWindow::pointer window = SimpleWindow::New();
	window->addRenderer(imageRenderer);
	window->addRenderer(segmentationRenderer);
	window->set2DMode();
	window->start();

}
