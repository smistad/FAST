#include "DummyIGTLServer.hpp"
#include "FAST/Testing.hpp"
#include "FAST/Streamers/OpenIGTLinkStreamer.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"
#include "FAST/Algorithms/AddTransformation/AddTransformation.hpp"

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
    addTransformation->setInputConnection(streamer->getOutputPort<Image>("DummyImage"));
    addTransformation->setTransformationInputConnection(streamer->getOutputPort<AffineTransformation>("DummyTransform"));

    auto renderer = ImageRenderer::New();
    renderer->addInputConnection(addTransformation->getOutputPort());
    auto window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->setTimeout(5000);
    CHECK_NOTHROW(window->start());
}
