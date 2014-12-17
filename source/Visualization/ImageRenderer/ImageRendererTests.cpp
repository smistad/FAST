#include "ImageFileStreamer.hpp"
#include "catch.hpp"
#include "ImageFileImporter.hpp"
#include "ImageRenderer.hpp"
#include "SimpleWindow.hpp"

using namespace fast;

TEST_CASE("ImageRenderer with single 2D image", "[fast][ImageRenderer][visual]") {
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR)+"US-2D.jpg");
        ImageRenderer::pointer renderer = ImageRenderer::New();
        renderer->addInput(importer->getOutput());
        SimpleWindow::pointer window = SimpleWindow::New();
        window->addRenderer(renderer);
        window->set2DMode();
        window->setTimeout(500);

    CHECK_NOTHROW(window->start();
    );
}

TEST_CASE("ImageRenderer with dynamic 2D image", "[fast][ImageRenderer][visual]") {
    ImageFileStreamer::pointer streamer = ImageFileStreamer::New();
    streamer->setFilenameFormat(std::string(FAST_TEST_DATA_DIR)+"US-2Dt/US-2Dt_#.mhd");
        ImageRenderer::pointer renderer = ImageRenderer::New();
        renderer->addInput(streamer->getOutput());
        SimpleWindow::pointer window = SimpleWindow::New();
        window->addRenderer(renderer);
        window->set2DMode();
        window->setTimeout(1000);
    CHECK_NOTHROW(window->start();
    );
}
