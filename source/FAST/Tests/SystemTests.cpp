#include "FAST/Visualization/TriangleRenderer/TriangleRenderer.hpp"
#include "FAST/Streamers/ImageFileStreamer.hpp"
#include "FAST/Testing.hpp"
#include "FAST/Importers/MetaImageImporter.hpp"
#include "FAST/Algorithms/GaussianSmoothing/GaussianSmoothing.hpp"
#include "FAST/Visualization/SliceRenderer/SliceRenderer.hpp"
#include "FAST/Algorithms/SurfaceExtraction/SurfaceExtraction.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"
#include "FAST/DeviceManager.hpp"
#include "FAST/Config.hpp"
#include <FAST/Importers/ImageFileImporter.hpp>
#include <FAST/Algorithms/NonLocalMeans/NonLocalMeans.hpp>

using namespace fast;
TEST_CASE("Simple pipeline with ImageFileStreamer, GaussianSmoothing and ImageRenderer", "[fast][SystemTests][visual]") {
    ImageFileStreamer::pointer streamer = ImageFileStreamer::New();
    streamer->setFilenameFormat(Config::getTestDataPath() + "US/CarotidArtery/Right/US-2D_#.mhd");

    GaussianSmoothing::pointer filter = GaussianSmoothing::New();
    filter->setInputConnection(streamer->getOutputPort());
    filter->setMaskSize(3);
    filter->setStandardDeviation(2.0);

    ImageRenderer::pointer renderer = ImageRenderer::New();
    renderer->addInputConnection(filter->getOutputPort());
    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->setTimeout(10*1000);
    window->set2DMode();
    CHECK_NOTHROW(
        window->start();
    );
}
TEST_CASE("Simple pipeline with ImageFileStreamer, GaussianSmoothing and SliceRenderer on OpenCL device", "[fast][SystemTests][visual]") {
    ImageFileStreamer::pointer mhdStreamer = ImageFileStreamer::New();
    mhdStreamer->setFilenameFormat(Config::getTestDataPath()+"US/Ball/US-3Dt_#.mhd");

    GaussianSmoothing::pointer filter = GaussianSmoothing::New();
    filter->setInputConnection(mhdStreamer->getOutputPort());
    filter->setMaskSize(3);
    filter->setStandardDeviation(2.0);

    SliceRenderer::pointer renderer = SliceRenderer::New();
    renderer->addInputConnection(filter->getOutputPort());

    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->setTimeout(10*1000); // timeout after 10 seconds
    CHECK_NOTHROW(
    window->start();
    );
}

TEST_CASE("Simple pipeline with ImageFileStreamer, GaussianSmoothing, SurfaceExtraction and TriangleRenderer on OpenCL device", "[fast][SystemTests][visual]") {
    ImageFileStreamer::pointer mhdStreamer = ImageFileStreamer::New();
    mhdStreamer->setFilenameFormat(Config::getTestDataPath()+"US/Ball/US-3Dt_#.mhd");

    GaussianSmoothing::pointer filter = GaussianSmoothing::New();
    filter->setInputConnection(mhdStreamer->getOutputPort());
    filter->setMaskSize(5);
    filter->setStandardDeviation(2.0);

    SurfaceExtraction::pointer extractor = SurfaceExtraction::create();
    extractor->setInputConnection(filter->getOutputPort());
    extractor->setThreshold(200);

    TriangleRenderer::pointer renderer = TriangleRenderer::New();
    renderer->addInputConnection(extractor->getOutputPort());

    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->setTimeout(10*1000); // timeout after 10 seconds
    CHECK_NOTHROW(
    window->start();
    );
}

TEST_CASE("Simple pipeline with ImageFileStreamer, GaussianSmoothing and SliceRenderer on Host", "[fast][SystemTests][visual]") {
    ExecutionDevice::pointer host = Host::getInstance();
    ImageFileStreamer::pointer mhdStreamer = ImageFileStreamer::New();
    mhdStreamer->setFilenameFormat(Config::getTestDataPath()+"US/Ball/US-3Dt_#.mhd");
    mhdStreamer->setMainDevice(host);

    GaussianSmoothing::pointer filter = GaussianSmoothing::New();
    filter->setInputConnection(mhdStreamer->getOutputPort());
    filter->setMaskSize(3);
    filter->setStandardDeviation(2.0);
    filter->setMainDevice(host);

    SliceRenderer::pointer renderer = SliceRenderer::New();
    renderer->addInputConnection(filter->getOutputPort());

    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->setTimeout(10*1000); // timeout after 10 seconds
    CHECK_NOTHROW(
    window->start();
    );
}

TEST_CASE("v4 semantics", "[fast]") {
    auto importer = ImageFileImporter::create(Config::getTestDataPath() + "US/US-2D.jpg");

    auto nonLocalMeans = NonLocalMeans::create(5, 9, 0.2)->connect(importer);
}