#include "ImageFileStreamer.hpp"
#include "catch.hpp"
#include "ImageImporter.hpp"
#include "ImageRenderer.hpp"
#include "SimpleWindow.hpp"

using namespace fast;

TEST_CASE("ImageRenderer with single 2D image", "[fast][ImageRenderer][visual]") {
    ImageImporter::pointer importer = ImageImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR)+"US-2D.jpg");
    CHECK_NOTHROW(
        ImageRenderer::pointer renderer = ImageRenderer::New();
        renderer->setInput(importer->getOutput());
        SimpleWindow::pointer window = SimpleWindow::New();
        window->addRenderer(renderer);
        window->setTimeout(500);
        window->runMainLoop();
    );
}

TEST_CASE("ImageRenderer with dynamic 2D image", "[fast][ImageRenderer][visual]") {
    ImageFileStreamer::pointer streamer = ImageFileStreamer::New();
    streamer->setFilenameFormat(std::string(FAST_TEST_DATA_DIR)+"US-2Dt/US-2Dt_#.mhd");
    CHECK_NOTHROW(
        ImageRenderer::pointer renderer = ImageRenderer::New();
        renderer->setInput(streamer->getOutput());
        SimpleWindow::pointer window = SimpleWindow::New();
        window->addRenderer(renderer);
        window->setTimeout(1000);
        window->runMainLoop();
    );
}
