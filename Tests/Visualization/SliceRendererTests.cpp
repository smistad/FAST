#include "catch.hpp"
#include "MetaImageStreamer.hpp"
#include "SliceRenderer.hpp"
#include "SimpleWindow.hpp"

using namespace fast;

TEST_CASE("SliceRenderer with no parameters set", "[fast][SliceRenderer]") {
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

TEST_CASE("SliceRenderer with slice plane X set", "[fast][SliceRenderer]") {
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

TEST_CASE("SliceRenderer with slice plane Y set", "[fast][SliceRenderer]") {
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

TEST_CASE("SliceRenderer with slice plane Z set", "[fast][SliceRenderer]") {
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
