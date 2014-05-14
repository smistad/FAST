#include "catch.hpp"
#include "MetaImageStreamer.hpp"
#include "SliceRenderer.hpp"
#include "SimpleWindow.hpp"

using namespace fast;

TEST_CASE("SliceRenderer", "[fast][SliceRenderer]") {
    CHECK_NOTHROW(
        MetaImageStreamer::pointer mhdStreamer = MetaImageStreamer::New();
        mhdStreamer->setFilenameFormat(std::string(FAST_ROOT_DIR)+"TestData/US-3Dt/US-3Dt_#.mhd");
        SliceRenderer::pointer renderer = SliceRenderer::New();
        renderer->setInput(mhdStreamer->getOutput());
        SimpleWindow::pointer window = SimpleWindow::New();
        window->addRenderer(renderer);

        // TODO should run main loop for a few seconds
    );
}
