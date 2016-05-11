#include "FAST/Testing.hpp"
#include "UltrasoundVesselDetection.hpp"
#include "FAST/Streamers/ImageFileStreamer.hpp"
#include "FAST/Data/Image.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SegmentationRenderer/SegmentationRenderer.hpp"

using namespace fast;

TEST_CASE("UltrasoundVesselDetection", "[fast][UltrasoundVesselDetection][visual]") {
	std::vector<std::string> recordings = {
	"/home/smistad/AssistantTestData/1/US-Acq_03_20150608T103739/Acquisition/US-Acq_03_20150608T103739_Image_Transducer_#.mhd",
	"/home/smistad/AssistantTestData/1/US-Acq_04_20150608T103837/Acquisition/US-Acq_04_20150608T103837_Image_Transducer_#.mhd",
	"/home/smistad/AssistantTestData/1/US-Acq_07_20150608T104148/Acquisition/US-Acq_07_20150608T104148_Image_Transducer_#.mhd",
	"/home/smistad/AssistantTestData/1/US-Acq_08_20150608T104238/Acquisition/US-Acq_08_20150608T104238_Image_Transducer_#.mhd",
	"/home/smistad/AssistantTestData/0/US-Acq_03_20150608T102144/Acquisition/US-Acq_03_20150608T102144_Image_Transducer_#.mhd",
	"/home/smistad/AssistantTestData/0/US-Acq_04_20150608T102242/Acquisition/US-Acq_04_20150608T102242_Image_Transducer_#.mhd",
	"/home/smistad/AssistantTestData/0/US-Acq_07_20150608T102703/Acquisition/US-Acq_07_20150608T102703_Image_Transducer_#.mhd",
	"/home/smistad/AssistantTestData/0/US-Acq_08_20150608T102854/Acquisition/US-Acq_08_20150608T102854_Image_Transducer_#.mhd",
	"/home/smistad/AssistantTestData/2/US-Acq_03_20150608T104805/Acquisition/US-Acq_03_20150608T104805_Image_Transducer_#.mhd",
	"/home/smistad/AssistantTestData/2/US-Acq_04_20150608T104910/Acquisition/US-Acq_04_20150608T104910_Image_Transducer_#.mhd",
	"/home/smistad/AssistantTestData/2/US-Acq_07_20150608T105549/Acquisition/US-Acq_07_20150608T105549_Image_Transducer_#.mhd",
	"/home/smistad/AssistantTestData/2/US-Acq_08_20150608T105649/Acquisition/US-Acq_08_20150608T105649_Image_Transducer_#.mhd",
	"/home/smistad/AssistantTestData/3/US-Acq_03_20150608T113646/Acquisition/US-Acq_03_20150608T113646_Image_Transducer_#.mhd",
	"/home/smistad/AssistantTestData/3/US-Acq_04_20150608T113750/Acquisition/US-Acq_04_20150608T113750_Image_Transducer_#.mhd",
	"/home/smistad/AssistantTestData/3/US-Acq_07_20150608T114115/Acquisition/US-Acq_07_20150608T114115_Image_Transducer_#.mhd",
	"/home/smistad/AssistantTestData/3/US-Acq_08_20150608T114217/Acquisition/US-Acq_08_20150608T114217_Image_Transducer_#.mhd",
	"/home/smistad/AssistantTestData/4/US-Acq_03_20150608T112610/Acquisition/US-Acq_03_20150608T112610_Image_Transducer_#.mhd",
	"/home/smistad/AssistantTestData/4/US-Acq_04_20150608T112656/Acquisition/US-Acq_04_20150608T112656_Image_Transducer_#.mhd",
	"/home/smistad/AssistantTestData/4/US-Acq_08_20150608T113129/Acquisition/US-Acq_08_20150608T113129_Image_Transducer_#.mhd",
	"/home/smistad/AssistantTestData/4/US-Acq_09_20150608T113245/Acquisition/US-Acq_09_20150608T113245_Image_Transducer_#.mhd",
	"/home/smistad/AssistantTestData/5/US-Acq_03_20150608T111610/Acquisition/US-Acq_03_20150608T111610_Image_Transducer_#.mhd",
	"/home/smistad/AssistantTestData/5/US-Acq_04_20150608T111701/Acquisition/US-Acq_04_20150608T111701_Image_Transducer_#.mhd",
	"/home/smistad/AssistantTestData/5/US-Acq_07_20150608T111940/Acquisition/US-Acq_07_20150608T111940_Image_Transducer_#.mhd",
	"/home/smistad/AssistantTestData/6/0/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/6/1/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/6/2/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/6/3/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/6/4/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/7/0/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/7/1/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/7/2/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/7/3/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/8/0/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/8/1/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/8/2/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/8/3/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/9/0/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/9/1/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/9/2/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/9/3/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/10/0/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/10/1/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/10/2/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/10/3/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/11/0/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/11/1/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/11/2/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/11/3/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/12/0/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/12/1/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/12/2/US-2D_#.mhd",
	"/home/smistad/AssistantTestData/12/3/US-2D_#.mhd"
	};
	ImageFileStreamer::pointer streamer = ImageFileStreamer::New();
	streamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
	streamer->setFilenameFormat(recordings[0]);
	streamer->update();

	UltrasoundVesselDetection::pointer detector = UltrasoundVesselDetection::New();
	detector->setInputConnection(streamer->getOutputPort());
	detector->enableRuntimeMeasurements();

	SegmentationRenderer::pointer segmentationRenderer = SegmentationRenderer::New();
	segmentationRenderer->addInputConnection(detector->getOutputSegmentationPort());
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
