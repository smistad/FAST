#include "FAST/Streamers/Tests/DummyIGTLServer.hpp"
#include "FAST/Streamers/IGTLinkStreamer.hpp"
#include "FAST/Algorithms/NeuralNetwork/ImageClassifier.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"

using namespace fast;

int main() {

    Reporter::setGlobalReportMethod(Reporter::COUT);

    /*
    // Start dummy IGT link server
    ImageFileStreamer::pointer fileStreamer = ImageFileStreamer::New();
    fileStreamer->setFilenameFormats({
            Config::getTestDataPath() + "US/Heart/ApicalTwoChamber/US-2D_#.mhd",
            Config::getTestDataPath() + "US/Heart/ApicalFourChamber/US-2D_#.mhd",
    });
    fileStreamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
    DummyIGTLServer server;
    server.setImageStreamer(fileStreamer);
    server.setPort(18944);
    server.start();
     */

    // Start IGTLink streamer
    IGTLinkStreamer::pointer streamer = IGTLinkStreamer::New();
    streamer->setConnectionAddress("localhost");
    streamer->setConnectionPort(18944);

    ImageClassifier::pointer classifier = ImageClassifier::New();
    classifier->setScaleFactor(1.0f/255.0f);
    classifier->load("/home/smistad/Downloads/cvc_net");
    classifier->setInputSize(128,128);
    classifier->setOutputParameters({"Softmax"});
    classifier->setLabels({
                              "Parasternal short axis",
                              "Parasternal long axis",
                              "Apical two-chamber",
                              "Apical four-chamber",
                              "Apical long axis"
                      });
    classifier->setInputConnection(streamer->getOutputPort<Image>("tissue"));
    classifier->enableRuntimeMeasurements();

    ClassificationToText::pointer classToText = ClassificationToText::New();
    classToText->setInputConnection(classifier->getOutputPort());

    ImageRenderer::pointer renderer = ImageRenderer::New();
    renderer->setInputConnection(streamer->getOutputPort<Image>("tissue"));

    SimpleWindow::pointer window = SimpleWindow::New();

    TextRenderer::pointer textRenderer = TextRenderer::New();
    textRenderer->setView(window->getView());
    textRenderer->setPosition(Vector2i(10, 40));
    textRenderer->setFontSize(32);
    textRenderer->setInputConnection(classToText->getOutputPort());

    window->addRenderer(renderer);
    window->addRenderer(textRenderer);
    window->setSize(1024, 1024);
    //window->enableFullscreen();
    window->set2DMode();
    window->getView()->setBackgroundColor(Color::Black());
    window->start();

    classifier->getRuntime()->print();
    classifier->getRuntime("input_data_copy")->print();
    classifier->getRuntime("network_execution")->print();
}