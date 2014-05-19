#include "catch.hpp"
#include "MetaImageStreamer.hpp"
#include "SurfaceRenderer.hpp"
#include "SimpleWindow.hpp"

using namespace fast;

TEST_CASE("SurfaceRenderer", "[fast][SurfaceRenderer]") {
    CHECK_NOTHROW(
        MetaImageStreamer::pointer mhdStreamer = MetaImageStreamer::New();
        mhdStreamer->setFilenameFormat(std::string(FAST_ROOT_DIR)+"TestData/US-3Dt/US-3Dt_#.mhd");
        SurfaceRenderer::pointer renderer = SurfaceRenderer::New();
        renderer->setInput(mhdStreamer->getOutput());
        renderer->setThreshold(200);
        SimpleWindow::pointer window = SimpleWindow::New();
        window->addRenderer(renderer);
        window->setTimeout(1000);
        window->runMainLoop();
    );
}
