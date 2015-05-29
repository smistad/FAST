#include "FAST/Tests/catch.hpp"
#include "FAST/Streamers/ImageIGTLinkStreamer.hpp"
#include "FAST/Streamers/Tests/DummyIGTLImageServer.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"

using namespace fast;

TEST_CASE("Stream 2D images using ImageIGTLinkStreamer", "[ImageIGTLinkStreamer][fast][IGTLink][visual]") {

    ImageFileStreamer::pointer fileStreamer = ImageFileStreamer::New();
    fileStreamer->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-2Dt/US-2Dt_#.mhd");
    fileStreamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
    DummyIGTLImageServer server;
    server.setImageStreamer(fileStreamer);
    server.setPort(18944);
    server.start();

    ImageIGTLinkStreamer::pointer streamer = ImageIGTLinkStreamer::New();
    streamer->setConnectionAddress("localhost");
    streamer->setConnectionPort(18944);

    ImageRenderer::pointer renderer = ImageRenderer::New();
    renderer->addInputConnection(streamer->getOutputPort());
    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->setTimeout(5000);
    window->start();
}
