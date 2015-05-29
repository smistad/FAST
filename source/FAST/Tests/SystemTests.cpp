#include "FAST/Visualization/MeshRenderer/MeshRenderer.hpp"
#include "FAST/Streamers/ImageFileStreamer.hpp"
#include "FAST/Tests/catch.hpp"
#include "FAST/Importers/MetaImageImporter.hpp"
#include "FAST/Algorithms/GaussianSmoothingFilter/GaussianSmoothingFilter.hpp"
#include "FAST/Visualization/SliceRenderer/SliceRenderer.hpp"
#include "FAST/Algorithms/SurfaceExtraction/SurfaceExtraction.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"
#include "FAST/DeviceManager.hpp"

using namespace fast;
TEST_CASE("Simple pipeline with ImageFileStreamer, GaussianSmoothingFilter and ImageRenderer", "[fast][SystemTests][visual]") {
    ImageFileStreamer::pointer streamer = ImageFileStreamer::New();
    streamer->setFilenameFormat(std::string(FAST_TEST_DATA_DIR)+"US-2Dt/US-2Dt_#.mhd");
    streamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);

    GaussianSmoothingFilter::pointer filter = GaussianSmoothingFilter::New();
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
TEST_CASE("Simple pipeline with ImageFileStreamer, GaussianSmoothingFilter and SliceRenderer on OpenCL device", "[fast][SystemTests][visual]") {
    ImageFileStreamer::pointer mhdStreamer = ImageFileStreamer::New();
    mhdStreamer->setFilenameFormat(std::string(FAST_TEST_DATA_DIR)+"/US-3Dt/US-3Dt_#.mhd");
    mhdStreamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);

    GaussianSmoothingFilter::pointer filter = GaussianSmoothingFilter::New();
    filter->setInputConnection(mhdStreamer->getOutputPort());
    filter->setMaskSize(3);
    filter->setStandardDeviation(2.0);

    SliceRenderer::pointer renderer = SliceRenderer::New();
    renderer->setInputConnection(filter->getOutputPort());

    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->setTimeout(10*1000); // timeout after 10 seconds
    CHECK_NOTHROW(
    window->start();
    );
}

TEST_CASE("Simple pipeline with ImageFileStreamer, GaussianSmoothingFilter, SurfaceExtraction and MeshRenderer on OpenCL device", "[fast][SystemTests][visual]") {
    ImageFileStreamer::pointer mhdStreamer = ImageFileStreamer::New();
    mhdStreamer->setFilenameFormat(std::string(FAST_TEST_DATA_DIR)+"/US-3Dt/US-3Dt_#.mhd");
    mhdStreamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);

    GaussianSmoothingFilter::pointer filter = GaussianSmoothingFilter::New();
    filter->setInputConnection(mhdStreamer->getOutputPort());
    filter->setMaskSize(5);
    filter->setStandardDeviation(2.0);

    SurfaceExtraction::pointer extractor = SurfaceExtraction::New();
    extractor->setInputConnection(filter->getOutputPort());
    extractor->setThreshold(200);

    MeshRenderer::pointer renderer = MeshRenderer::New();
    renderer->addInputConnection(extractor->getOutputPort());

    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->setTimeout(10*1000); // timeout after 10 seconds
    CHECK_NOTHROW(
    window->start();
    );
}

TEST_CASE("Simple pipeline with ImageFileStreamer, GaussianSmoothingFilter and SliceRenderer on Host", "[fast][SystemTests][visual]") {
    ExecutionDevice::pointer host = Host::getInstance();
    ImageFileStreamer::pointer mhdStreamer = ImageFileStreamer::New();
    mhdStreamer->setFilenameFormat(std::string(FAST_TEST_DATA_DIR)+"/US-3Dt/US-3Dt_#.mhd");
    mhdStreamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
    mhdStreamer->setMainDevice(host);

    GaussianSmoothingFilter::pointer filter = GaussianSmoothingFilter::New();
    filter->setInputConnection(mhdStreamer->getOutputPort());
    filter->setMaskSize(3);
    filter->setStandardDeviation(2.0);
    filter->setMainDevice(host);

    SliceRenderer::pointer renderer = SliceRenderer::New();
    renderer->setInputConnection(filter->getOutputPort());

    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->setTimeout(10*1000); // timeout after 10 seconds
    CHECK_NOTHROW(
    window->start();
    );
}
