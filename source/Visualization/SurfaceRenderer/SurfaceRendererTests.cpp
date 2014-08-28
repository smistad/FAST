#include "catch.hpp"
#include "MetaImageStreamer.hpp"
#include "SurfaceRenderer.hpp"
#include "SurfaceExtraction.hpp"
#include "SimpleWindow.hpp"
#include "VTKSurfaceFileImporter.hpp"

using namespace fast;

TEST_CASE("SurfaceRenderer on LV surface model", "[fast][SurfaceRenderer]") {
    CHECK_NOTHROW(
        VTKSurfaceFileImporter::pointer importer = VTKSurfaceFileImporter::New();
        importer->setFilename(std::string(FAST_TEST_DATA_DIR) + "Surface_LV.vtk");
        SurfaceRenderer::pointer renderer = SurfaceRenderer::New();
        renderer->setInput(importer->getOutput());
        SimpleWindow::pointer window = SimpleWindow::New();
        window->addRenderer(renderer);
        window->setTimeout(1000);
        window->runMainLoop();
    );
}

TEST_CASE("SurfaceRenderer on stream of surfaces", "[fast][SurfaceRenderer]") {
    CHECK_NOTHROW(
        MetaImageStreamer::pointer mhdStreamer = MetaImageStreamer::New();
        mhdStreamer->setFilenameFormat(std::string(FAST_ROOT_DIR)+"TestData/US-3Dt/US-3Dt_#.mhd");
        SurfaceExtraction::pointer extractor = SurfaceExtraction::New();
        extractor->setInput(mhdStreamer->getOutput());
        extractor->setThreshold(200);
        SurfaceRenderer::pointer renderer = SurfaceRenderer::New();
        renderer->setInput(extractor->getOutput());
        SimpleWindow::pointer window = SimpleWindow::New();
        window->addRenderer(renderer);
        window->setTimeout(1000);
        window->runMainLoop();
    );
}
