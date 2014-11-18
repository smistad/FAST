#include "catch.hpp"
#include "MetaImageImporter.hpp"
#include "MetaImageStreamer.hpp"
#include "GaussianSmoothingFilter.hpp"
#include "SliceRenderer.hpp"
#include "SurfaceRenderer.hpp"
#include "SurfaceExtraction.hpp"
#include "ImageRenderer.hpp"
#include "SimpleWindow.hpp"
#include "DeviceManager.hpp"

using namespace fast;
TEST_CASE("Simple pipeline with MetaImageStreamer, GaussianSmoothingFilter and ImageRenderer", "[fast][SystemTests][visual]") {
    MetaImageStreamer::pointer streamer = MetaImageStreamer::New();
    streamer->setFilenameFormat(std::string(FAST_TEST_DATA_DIR)+"US-2Dt/US-2Dt_#.mhd");
    streamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);

    GaussianSmoothingFilter::pointer filter = GaussianSmoothingFilter::New();
    filter->setInput(streamer->getOutput());
    filter->setMaskSize(3);
    filter->setStandardDeviation(2.0);

    ImageRenderer::pointer renderer = ImageRenderer::New();
    std::cout << "asdasd" << std::endl;
    renderer->setInput(filter->getOutput());
    std::cout << "asdasd" << std::endl;
    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->setTimeout(10*1000);
    window->set2DMode();
    CHECK_NOTHROW(
        window->runMainLoop();
    );
}
TEST_CASE("Simple pipeline with MetaImageStreamer, GaussianSmoothingFilter and SliceRenderer on OpenCL device", "[fast][SystemTests][visual]") {
    MetaImageStreamer::pointer mhdStreamer = MetaImageStreamer::New();
    mhdStreamer->setFilenameFormat(std::string(FAST_TEST_DATA_DIR)+"/US-3Dt/US-3Dt_#.mhd");
    mhdStreamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);

    GaussianSmoothingFilter::pointer filter = GaussianSmoothingFilter::New();
    filter->setInput(mhdStreamer->getOutput());
    filter->setMaskSize(3);
    filter->setStandardDeviation(2.0);

    SliceRenderer::pointer renderer = SliceRenderer::New();
    renderer->setInput(filter->getOutput());

    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->setTimeout(10*1000); // timeout after 10 seconds
    CHECK_NOTHROW(
    window->runMainLoop();
    );
}

TEST_CASE("Simple pipeline with MetaImageStreamer, GaussianSmoothingFilter, SurfaceExtraction and SurfaceRenderer on OpenCL device", "[fast][SystemTests][visual]") {
    MetaImageStreamer::pointer mhdStreamer = MetaImageStreamer::New();
    mhdStreamer->setFilenameFormat(std::string(FAST_TEST_DATA_DIR)+"/US-3Dt/US-3Dt_#.mhd");
    mhdStreamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);

    GaussianSmoothingFilter::pointer filter = GaussianSmoothingFilter::New();
    filter->setInput(mhdStreamer->getOutput());
    filter->setMaskSize(5);
    filter->setStandardDeviation(2.0);

    SurfaceExtraction::pointer extractor = SurfaceExtraction::New();
    extractor->setInput(filter->getOutput());
    extractor->setThreshold(200);

    SurfaceRenderer::pointer renderer = SurfaceRenderer::New();
    renderer->setInput(extractor->getOutput());

    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->setTimeout(10*1000); // timeout after 10 seconds
    CHECK_NOTHROW(
    window->runMainLoop();
    );
}

TEST_CASE("Simple pipeline with MetaImageStreamer, GaussianSmoothingFilter and SliceRenderer on Host", "[fast][SystemTests][visual]") {
    ExecutionDevice::pointer host = Host::New();
    MetaImageStreamer::pointer mhdStreamer = MetaImageStreamer::New();
    mhdStreamer->setFilenameFormat(std::string(FAST_TEST_DATA_DIR)+"/US-3Dt/US-3Dt_#.mhd");
    mhdStreamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
    mhdStreamer->setDevice(host);

    GaussianSmoothingFilter::pointer filter = GaussianSmoothingFilter::New();
    filter->setInput(mhdStreamer->getOutput());
    filter->setMaskSize(3);
    filter->setStandardDeviation(2.0);
    filter->setMainDevice(host);

    SliceRenderer::pointer renderer = SliceRenderer::New();
    renderer->setInput(filter->getOutput());

    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->setTimeout(10*1000); // timeout after 10 seconds
    CHECK_NOTHROW(
    window->runMainLoop();
    );
}
