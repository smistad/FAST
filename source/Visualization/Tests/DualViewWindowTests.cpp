#include "catch.hpp"
#include "ImageFileImporter.hpp"
#include "ImageRenderer.hpp"
#include "DualViewWindow.hpp"
#include "MeshRenderer.hpp"
#include "ImageFileStreamer.hpp"
#include "GaussianSmoothingFilter.hpp"
#include "SurfaceExtraction.hpp"

using namespace fast;

TEST_CASE("DualViewWindow", "[fast][DualViewWindow][visual]") {
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR)+"US-2D.jpg");

    ImageRenderer::pointer renderer = ImageRenderer::New();
    renderer->setInputConnection(importer->getOutputPort());

    ImageFileStreamer::pointer mhdStreamer = ImageFileStreamer::New();
    mhdStreamer->setFilenameFormat(std::string(FAST_TEST_DATA_DIR)+"/US-3Dt/US-3Dt_#.mhd");
    mhdStreamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);

    SurfaceExtraction::pointer extractor = SurfaceExtraction::New();
    extractor->setInputConnection(mhdStreamer->getOutputPort());
    extractor->setThreshold(200);

    MeshRenderer::pointer renderer2 = MeshRenderer::New();
    renderer2->addInputConnection(extractor->getOutputPort());

    DualViewWindow::pointer window = DualViewWindow::New();
    window->addRendererToBottomRightView(renderer);
    window->addRendererToTopLeftView(renderer2);
    //window->setTimeout(2000);

    CHECK_NOTHROW(window->start());
}
