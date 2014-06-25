#include "catch.hpp"
#include "MetaImageStreamer.hpp"
#include "SurfaceRenderer.hpp"
#include "SurfaceExtraction.hpp"
#include "SimpleWindow.hpp"

using namespace fast;

TEST_CASE("SurfaceRenderer", "[fast][SurfaceRenderer]") {
    CHECK_NOTHROW(
        MetaImageStreamer::pointer mhdStreamer = MetaImageStreamer::New();
        mhdStreamer->setFilenameFormat(std::string(FAST_ROOT_DIR)+"TestData/US-3Dt/US-3Dt_#.mhd");
        SurfaceExtraction::pointer extractor = SurfaceExtraction::New();
        extractor->setInput(mhdStreamer->getOutput());
        extractor->setThreshold(200);
        SurfaceRenderer::pointer renderer = SurfaceRenderer::New();
        renderer->setInput(extractor->getOutput());
        SimpleWindow::pointer window = SimpleWindow::New();
        window->addRenderer(renderer);
        window->setTimeout(1000);
        window->runMainLoop();
    );
}
