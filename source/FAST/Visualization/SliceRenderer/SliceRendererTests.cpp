#include "FAST/Streamers/ImageFileStreamer.hpp"
#include "FAST/Tests/catch.hpp"
#include "FAST/Importers/MetaImageImporter.hpp"
#include "FAST/Visualization/SliceRenderer/SliceRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"

using namespace fast;

TEST_CASE("SliceRenderer with no input throws exception", "[fast][SliceRenderer]") {
    SliceRenderer::pointer renderer = SliceRenderer::New();
    CHECK_THROWS(renderer->update());
}

TEST_CASE("SliceRenderer on static data with no parameters set", "[fast][SliceRenderer][visual]") {
    MetaImageImporter::pointer importer = MetaImageImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR)+"US-3Dt/US-3Dt_0.mhd");
    CHECK_NOTHROW(
        SliceRenderer::pointer renderer = SliceRenderer::New();
        renderer->setInputConnection(importer->getOutputPort());
        SimpleWindow::pointer window = SimpleWindow::New();
        window->addRenderer(renderer);
        window->setTimeout(500);
        window->start();
    );
}

TEST_CASE("SliceRenderer on dynamic data with no parameters set", "[fast][SliceRenderer][visual]") {
    ImageFileStreamer::pointer mhdStreamer = ImageFileStreamer::New();
    mhdStreamer->setFilenameFormat(std::string(FAST_TEST_DATA_DIR)+"US-3Dt/US-3Dt_#.mhd");
    CHECK_NOTHROW(
        SliceRenderer::pointer renderer = SliceRenderer::New();
        renderer->setInputConnection(mhdStreamer->getOutputPort());
        SimpleWindow::pointer window = SimpleWindow::New();
        window->addRenderer(renderer);
        window->setTimeout(1000);
        window->start();
    );
}

TEST_CASE("SliceRenderer on dynamic data with slice plane X set", "[fast][SliceRenderer][visual]") {
    ImageFileStreamer::pointer mhdStreamer = ImageFileStreamer::New();
    mhdStreamer->setFilenameFormat(std::string(FAST_TEST_DATA_DIR)+"US-3Dt/US-3Dt_#.mhd");
    CHECK_NOTHROW(
        SliceRenderer::pointer renderer = SliceRenderer::New();
        renderer->setInputConnection(mhdStreamer->getOutputPort());
        renderer->setSlicePlane(PLANE_X);
        SimpleWindow::pointer window = SimpleWindow::New();
        window->addRenderer(renderer);
        window->setTimeout(1000);
        window->start();
    );
}

TEST_CASE("SliceRenderer on dynamic data with slice plane Y set", "[fast][SliceRenderer][visual]") {
    ImageFileStreamer::pointer mhdStreamer = ImageFileStreamer::New();
    mhdStreamer->setFilenameFormat(std::string(FAST_TEST_DATA_DIR)+"US-3Dt/US-3Dt_#.mhd");
    CHECK_NOTHROW(
        SliceRenderer::pointer renderer = SliceRenderer::New();
        renderer->setInputConnection(mhdStreamer->getOutputPort());
        renderer->setSlicePlane(PLANE_Y);
        SimpleWindow::pointer window = SimpleWindow::New();
        window->addRenderer(renderer);
        window->setTimeout(1000);
        window->start();
    );
}

TEST_CASE("SliceRenderer on dynamic data with slice plane Z set", "[fast][SliceRenderer][visual]") {
    ImageFileStreamer::pointer mhdStreamer = ImageFileStreamer::New();
    mhdStreamer->setFilenameFormat(std::string(FAST_TEST_DATA_DIR)+"US-3Dt/US-3Dt_#.mhd");
    CHECK_NOTHROW(
        SliceRenderer::pointer renderer = SliceRenderer::New();
        renderer->setInputConnection(mhdStreamer->getOutputPort());
        renderer->setSlicePlane(PLANE_Z);
        SimpleWindow::pointer window = SimpleWindow::New();
        window->addRenderer(renderer);
        window->setTimeout(1000);
        window->start();
    );
}

TEST_CASE("Setting slice nr in SliceRenderer too high should not throw exception", "[fast][SliceRenderer][visual]") {
    ImageFileStreamer::pointer mhdStreamer = ImageFileStreamer::New();
    mhdStreamer->setFilenameFormat(std::string(FAST_TEST_DATA_DIR)+"US-3Dt/US-3Dt_#.mhd");
    CHECK_NOTHROW(
        SliceRenderer::pointer renderer = SliceRenderer::New();
        renderer->setInputConnection(mhdStreamer->getOutputPort());
        renderer->setSliceToRender(1000);
        SimpleWindow::pointer window = SimpleWindow::New();
        window->addRenderer(renderer);
        window->setTimeout(1000);
        window->start();
    );
}
