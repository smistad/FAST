#include "FAST/Testing.hpp"
#include "ImageClassificationNetwork.hpp"
#include "FAST/Streamers/ImageFileStreamer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"

using namespace fast;

/*
TEST_CASE("Image classifier", "[fast][ImageClassificationNetwork]") {

	Reporter::setGlobalReportMethod(Reporter::COUT);
	ImageFileStreamer::pointer importer = ImageFileStreamer::New();
    importer->setFilenameFormat("/media/extra/GRUE_images/Clinic001/F4AFP6A2/US-2D_#.png");

	ImageClassificationNetwork::pointer classifier = ImageClassificationNetwork::New();
	classifier->load("/home/smistad/Downloads/cvc_inc_merged");
    classifier->setInputSize(128,128);
	classifier->setOutputParameters({"Softmax"});
    classifier->setLabels({
        "Parasternal short axis",
        "Parasternal long axis",
        "Apical two-chamber",
        "Apical five-chamber",
        "Apical four-chamber",
        "Apical long axis"
    });
	classifier->setInputConnection(importer->getOutputPort());
	classifier->enableRuntimeMeasurements();

	//ImageRenderer::pointer renderer = ImageRenderer::New();
	//renderer->setInputConnection(streamer->getOutputPort());

	//SimpleWindow::pointer window = SimpleWindow::New();

	classifier->getRuntime()->print();
}
*/
