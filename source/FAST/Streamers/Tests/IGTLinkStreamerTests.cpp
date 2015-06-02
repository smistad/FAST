#include "DummyIGTLServer.hpp"
#include "FAST/Tests/catch.hpp"
#include "FAST/Streamers/IGTLinkStreamer.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"
#include "FAST/Algorithms/AddTransformation/AddTransformation.hpp"

using namespace fast;

TEST_CASE("Stream 2D images using IGTLinkStreamer", "[IGTLinkStreamer][fast][IGTLink][visual]") {

    ImageFileStreamer::pointer fileStreamer = ImageFileStreamer::New();
    fileStreamer->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-2Dt/US-2Dt_#.mhd");
    fileStreamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
    DummyIGTLServer server;
    server.setImageStreamer(fileStreamer);
    server.setPort(18944);
    server.start();

    IGTLinkStreamer::pointer streamer = IGTLinkStreamer::New();
    streamer->setConnectionAddress("localhost");
    streamer->setConnectionPort(18944);

    AddTransformation::pointer addTransformation = AddTransformation::New();
    addTransformation->setInputConnection(streamer->getOutputPort<Image>("DummyImage"));
    addTransformation->setTransformationInputConnection(streamer->getOutputPort<AffineTransformation>("DummyTransform"));

    ImageRenderer::pointer renderer = ImageRenderer::New();
    renderer->addInputConnection(addTransformation->getOutputPort());
    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->setTimeout(5000);
    CHECK_NOTHROW(window->start());
}
