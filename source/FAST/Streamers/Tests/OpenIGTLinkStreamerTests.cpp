#include "DummyIGTLServer.hpp"
#include "FAST/Testing.hpp"
#include "FAST/Streamers/OpenIGTLinkStreamer.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"
#include "FAST/Algorithms/AddTransformation/AddTransformation.hpp"
#include <FAST/Algorithms/Lambda/RunLambda.hpp>

using namespace fast;

TEST_CASE("Stream 2D images using OpenIGTLinkStreamer", "[OpenIGTLinkStreamer][fast][IGTLink][visual]") {

    auto fileStreamer = ImageFileStreamer::New();
    fileStreamer->setFilenameFormat(Config::getTestDataPath() + "US/CarotidArtery/Right/US-2D_#.mhd");
    DummyIGTLServer server;
    server.setImageStreamer(fileStreamer);
    server.setPort(18944);
    server.setMaximumFramesToSend(50);
    server.start();

    auto streamer = OpenIGTLinkStreamer::New();
    streamer->setConnectionAddress("localhost");
    streamer->setConnectionPort(18944);

    auto addTransformation = AddTransformation::New();
    addTransformation->setInputConnection(streamer->getOutputPort("DummyImage"));
    addTransformation->setInputConnection(1, streamer->getOutputPort("DummyTransform"));

    auto renderer = ImageRenderer::New();
    renderer->addInputConnection(addTransformation->getOutputPort());
    auto window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->setTimeout(5000);
    CHECK_NOTHROW(window->start());
}

/*
TEST_CASE("Stream image and string message using OpenIGTLinkStreamer", "[OpenIGTLinkStreamer][fast][IGTLink][visual]") {

    auto streamer = OpenIGTLinkStreamer::create("localhost");

    auto renderer = ImageRenderer::create();
    int bmode = streamer->getOutputPortNumber("BMode");
    int ecg = streamer->getOutputPortNumber("ECG");
    std::cout << "PORT numbers: " << bmode << " " << ecg << std::endl;
    renderer->connect(streamer,  bmode);

    auto runLambda = RunLambda::create([](DataObject::pointer data) {
        auto stringMessage = std::dynamic_pointer_cast<String>(data);
        if(stringMessage) {
            std::cout << "String message: " << stringMessage->get() << std::endl;
        }
        std::cout << "ECG output port gave: " << data->getNameOfClass() << std::endl;
        return DataList(data);
    })->connect(streamer, ecg);

    auto window = SimpleWindow2D::create()->connect(renderer);
    window->addProcessObject(runLambda);
    window->setTimeout(5000);
    CHECK_NOTHROW(window->start());
}
 */
