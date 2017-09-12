#include <FAST/Algorithms/ImageSlicer/ImageSlicer.hpp>
#include "FAST/Streamers/ImageFileStreamer.hpp"
#include "FAST/Testing.hpp"
#include "FAST/Importers/MetaImageImporter.hpp"
#include "FAST/Visualization/SliceRenderer/SliceRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"

using namespace fast;

TEST_CASE("SliceRenderer on static data with no parameters set", "[fast][SliceRenderer][visual]") {
    MetaImageImporter::pointer importer = MetaImageImporter::New();
    importer->setFilename(Config::getTestDataPath()+"US/Ball/US-3Dt_0.mhd");
    CHECK_NOTHROW(
        SliceRenderer::pointer renderer = SliceRenderer::New();
        renderer->addInputConnection(importer->getOutputPort());
        SimpleWindow::pointer window = SimpleWindow::New();
        window->addRenderer(renderer);
        window->setTimeout(500);
        window->start();
    );
}

TEST_CASE("SliceRenderer on dynamic data with no parameters set", "[fast][SliceRenderer][visual]") {
    ImageFileStreamer::pointer mhdStreamer = ImageFileStreamer::New();
    mhdStreamer->setFilenameFormat(Config::getTestDataPath()+"US/Ball/US-3Dt_#.mhd");
    CHECK_NOTHROW(
        SliceRenderer::pointer renderer = SliceRenderer::New();
        renderer->addInputConnection(mhdStreamer->getOutputPort());
        SimpleWindow::pointer window = SimpleWindow::New();
        window->addRenderer(renderer);
        window->setTimeout(1000);
        window->start();
    );
}

TEST_CASE("SliceRenderer on dynamic data with no parameters set, 2D view", "[fast][SliceRenderer][visual]") {
    ImageFileStreamer::pointer mhdStreamer = ImageFileStreamer::New();
    mhdStreamer->setFilenameFormat(Config::getTestDataPath()+"US/Ball/US-3Dt_#.mhd");
    CHECK_NOTHROW(
        SliceRenderer::pointer renderer = SliceRenderer::New();
        renderer->addInputConnection(mhdStreamer->getOutputPort(), PLANE_Z);
        SimpleWindow::pointer window = SimpleWindow::New();
        window->addRenderer(renderer);
        window->set2DMode();
        window->setTimeout(1000);
        window->start();
    );
}

TEST_CASE("SliceRenderer on dynamic data with custom slicer set", "[fast][SliceRenderer][visual]") {
    ImageFileStreamer::pointer mhdStreamer = ImageFileStreamer::New();
    mhdStreamer->setFilenameFormat(Config::getTestDataPath()+"US/Ball/US-3Dt_#.mhd");
    ImageSlicer::pointer slicer = ImageSlicer::New();
    slicer->setOrthogonalSlicePlane(PLANE_Y);
    CHECK_NOTHROW(
        SliceRenderer::pointer renderer = SliceRenderer::New();
        renderer->addInputConnection(mhdStreamer->getOutputPort(), slicer);
        SimpleWindow::pointer window = SimpleWindow::New();
        window->addRenderer(renderer);
        window->setTimeout(1000);
        window->start();
    );
}

TEST_CASE("SliceRenderer on dynamic data with slice plane X set", "[fast][SliceRenderer][visual]") {
    ImageFileStreamer::pointer mhdStreamer = ImageFileStreamer::New();
    mhdStreamer->setFilenameFormat(Config::getTestDataPath()+"US/Ball/US-3Dt_#.mhd");
    CHECK_NOTHROW(
        SliceRenderer::pointer renderer = SliceRenderer::New();
        renderer->addInputConnection(mhdStreamer->getOutputPort(), PLANE_X);
        SimpleWindow::pointer window = SimpleWindow::New();
        window->addRenderer(renderer);
        window->setTimeout(1000);
        window->start();
    );
}

TEST_CASE("SliceRenderer on dynamic data with slice plane Y set", "[fast][SliceRenderer][visual]") {
    ImageFileStreamer::pointer mhdStreamer = ImageFileStreamer::New();
    mhdStreamer->setFilenameFormat(Config::getTestDataPath()+"US/Ball/US-3Dt_#.mhd");
    CHECK_NOTHROW(
        SliceRenderer::pointer renderer = SliceRenderer::New();
        renderer->addInputConnection(mhdStreamer->getOutputPort(), PLANE_Y);
        SimpleWindow::pointer window = SimpleWindow::New();
        window->addRenderer(renderer);
        window->setTimeout(1000);
        window->start();
    );
}

TEST_CASE("SliceRenderer on dynamic data with slice plane Z set", "[fast][SliceRenderer][visual]") {
    ImageFileStreamer::pointer mhdStreamer = ImageFileStreamer::New();
    mhdStreamer->setFilenameFormat(Config::getTestDataPath()+"US/Ball/US-3Dt_#.mhd");
    CHECK_NOTHROW(
        SliceRenderer::pointer renderer = SliceRenderer::New();
        renderer->addInputConnection(mhdStreamer->getOutputPort(), PLANE_Z);
        SimpleWindow::pointer window = SimpleWindow::New();
        window->addRenderer(renderer);
        window->setTimeout(1000);
        window->start();
    );
}

TEST_CASE("SliceRenderer with arbitrary slice plane", "[fast][SliceRenderer][visual]") {
    MetaImageImporter::pointer importer = MetaImageImporter::New();
    importer->setFilename(Config::getTestDataPath()+"US/Ball/US-3Dt_0.mhd");
    CHECK_NOTHROW(
        SliceRenderer::pointer renderer = SliceRenderer::New();
        renderer->addInputConnection(importer->getOutputPort(), Plane(Vector3f(1, 0, 0)));
        SimpleWindow::pointer window = SimpleWindow::New();
        window->addRenderer(renderer);
        window->setTimeout(1000);
        window->start();
    );
}

TEST_CASE("Setting slice nr in SliceRenderer too high should not throw exception", "[fast][SliceRenderer][visual]") {
    ImageFileStreamer::pointer mhdStreamer = ImageFileStreamer::New();
    mhdStreamer->setFilenameFormat(Config::getTestDataPath()+"US/Ball/US-3Dt_#.mhd");
    CHECK_NOTHROW(
        SliceRenderer::pointer renderer = SliceRenderer::New();
        renderer->addInputConnection(mhdStreamer->getOutputPort(), PLANE_X, 1000);
        SimpleWindow::pointer window = SimpleWindow::New();
        window->addRenderer(renderer);
        window->setTimeout(1000);
        window->start();
    );
}
