#include "FAST/Streamers/ImageFileStreamer.hpp"
#include "FAST/Testing.hpp"
#include "FAST/Importers/ImageFileImporter.hpp"
#include "ImageRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"

using namespace fast;

TEST_CASE("ImageRenderer with single 2D image in 2D mode", "[fast][ImageRenderer][visual]") {
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(Config::getTestDataPath()+"US/US-2D.jpg");
    ImageRenderer::pointer renderer = ImageRenderer::New();
    renderer->setInputConnection(importer->getOutputPort());
    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->setSize(1024, 512);
    window->set2DMode();
    //window->setTimeout(500);

    CHECK_NOTHROW(window->start());
}

TEST_CASE("ImageRenderer with single 2D image in 2D mode (MHD)", "[fast][ImageRenderer][visual]") {
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(Config::getTestDataPath()+"US/CarotidArtery/Right/US-2D_0.mhd");
    ImageRenderer::pointer renderer = ImageRenderer::New();
    renderer->setInputConnection(importer->getOutputPort());
    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->setSize(1024, 512);
    window->set2DMode();
    //window->setTimeout(500);

    CHECK_NOTHROW(window->start());
}

TEST_CASE("ImageRenderer with dynamic 2D image in 2D mode", "[fast][ImageRenderer][visual]") {
    ImageFileStreamer::pointer streamer = ImageFileStreamer::New();
    streamer->setFilenameFormat(Config::getTestDataPath()+"US/CarotidArtery/Right/US-2D_#.mhd");
    ImageRenderer::pointer renderer = ImageRenderer::New();
    renderer->setInputConnection(streamer->getOutputPort());
    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->setSize(1024, 512);
    window->set2DMode();
    //window->setTimeout(1000);

    CHECK_NOTHROW(window->start());
}

TEST_CASE("ImageRenderer with dynamic 2D image in 3D mode", "[fast][ImageRenderer][visual]") {
    ImageFileStreamer::pointer streamer = ImageFileStreamer::New();
    streamer->setFilenameFormat(Config::getTestDataPath()+"US/CarotidArtery/Right/US-2D_#.mhd");
    ImageRenderer::pointer renderer = ImageRenderer::New();
    renderer->setInputConnection(streamer->getOutputPort());
    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->setTimeout(1000);

    CHECK_NOTHROW(window->start());
}
