#include "catch.hpp"
#include "MetaImageStreamer.hpp"
#include "MetaImageImporter.hpp"
#include "SliceRenderer.hpp"
#include "SimpleWindow.hpp"

using namespace fast;

TEST_CASE("SliceRenderer with no input throws exception", "[fast][SliceRenderer]") {
    SliceRenderer::pointer renderer = SliceRenderer::New();
    CHECK_THROWS(renderer->update());
}

TEST_CASE("SliceRenderer on static data with no parameters set", "[fast][SliceRenderer][visual]") {
    MetaImageImporter::pointer importer = MetaImageImporter::New();
    importer->setFilename(std::string(FAST_ROOT_DIR)+"TestData/US-3Dt/US-3Dt_0.mhd");
    CHECK_NOTHROW(
        SliceRenderer::pointer renderer = SliceRenderer::New();
        renderer->setInput(importer->getOutput());
        SimpleWindow::pointer window = SimpleWindow::New();
        window->addRenderer(renderer);
        window->setTimeout(500);
        window->runMainLoop();
    );
}

TEST_CASE("SliceRenderer on dynamic data with no parameters set", "[fast][SliceRenderer][visual]") {
    MetaImageStreamer::pointer mhdStreamer = MetaImageStreamer::New();
    mhdStreamer->setFilenameFormat(std::string(FAST_ROOT_DIR)+"TestData/US-3Dt/US-3Dt_#.mhd");
    CHECK_NOTHROW(
        SliceRenderer::pointer renderer = SliceRenderer::New();
        renderer->setInput(mhdStreamer->getOutput());
        SimpleWindow::pointer window = SimpleWindow::New();
        window->addRenderer(renderer);
        window->setTimeout(1000);
        window->runMainLoop();
    );
}

TEST_CASE("SliceRenderer on dynamic data with slice plane X set", "[fast][SliceRenderer][visual]") {
    MetaImageStreamer::pointer mhdStreamer = MetaImageStreamer::New();
    mhdStreamer->setFilenameFormat(std::string(FAST_ROOT_DIR)+"TestData/US-3Dt/US-3Dt_#.mhd");
    CHECK_NOTHROW(
        SliceRenderer::pointer renderer = SliceRenderer::New();
        renderer->setInput(mhdStreamer->getOutput());
        renderer->setSlicePlane(PLANE_X);
        SimpleWindow::pointer window = SimpleWindow::New();
        window->addRenderer(renderer);
        window->setTimeout(1000);
        window->runMainLoop();
    );
}

TEST_CASE("SliceRenderer on dynamic data with slice plane Y set", "[fast][SliceRenderer][visual]") {
    MetaImageStreamer::pointer mhdStreamer = MetaImageStreamer::New();
    mhdStreamer->setFilenameFormat(std::string(FAST_ROOT_DIR)+"TestData/US-3Dt/US-3Dt_#.mhd");
    CHECK_NOTHROW(
        SliceRenderer::pointer renderer = SliceRenderer::New();
        renderer->setInput(mhdStreamer->getOutput());
        renderer->setSlicePlane(PLANE_Y);
        SimpleWindow::pointer window = SimpleWindow::New();
        window->addRenderer(renderer);
        window->setTimeout(1000);
        window->runMainLoop();
    );
}

TEST_CASE("SliceRenderer on dynamic data with slice plane Z set", "[fast][SliceRenderer][visual]") {
    MetaImageStreamer::pointer mhdStreamer = MetaImageStreamer::New();
    mhdStreamer->setFilenameFormat(std::string(FAST_ROOT_DIR)+"TestData/US-3Dt/US-3Dt_#.mhd");
    CHECK_NOTHROW(
        SliceRenderer::pointer renderer = SliceRenderer::New();
        renderer->setInput(mhdStreamer->getOutput());
        renderer->setSlicePlane(PLANE_Z);
        SimpleWindow::pointer window = SimpleWindow::New();
        window->addRenderer(renderer);
        window->setTimeout(1000);
        window->runMainLoop();
    );
}

TEST_CASE("Setting slice nr in SliceRenderer too high should not throw exception", "[fast][SliceRenderer][visual]") {
    MetaImageStreamer::pointer mhdStreamer = MetaImageStreamer::New();
    mhdStreamer->setFilenameFormat(std::string(FAST_ROOT_DIR)+"TestData/US-3Dt/US-3Dt_#.mhd");
    CHECK_NOTHROW(
        SliceRenderer::pointer renderer = SliceRenderer::New();
        renderer->setInput(mhdStreamer->getOutput());
        renderer->setSliceToRender(1000);
        SimpleWindow::pointer window = SimpleWindow::New();
        window->addRenderer(renderer);
        window->setTimeout(1000);
        window->runMainLoop();
    );
}
