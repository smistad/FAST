#include "VTKMeshFileImporter.hpp"
#include "ImageFileStreamer.hpp"
#include "catch.hpp"
#include "MeshRenderer.hpp"
#include "SurfaceExtraction.hpp"
#include "SimpleWindow.hpp"

using namespace fast;

TEST_CASE("SurfaceRenderer on LV surface model", "[fast][MeshRenderer][visual]") {
    CHECK_NOTHROW(
        VTKMeshFileImporter::pointer importer = VTKMeshFileImporter::New();
        importer->setFilename(std::string(FAST_TEST_DATA_DIR) + "Surface_LV.vtk");
        MeshRenderer::pointer renderer = MeshRenderer::New();
        renderer->setInput(importer->getOutput());
        SimpleWindow::pointer window = SimpleWindow::New();
        window->addRenderer(renderer);
        window->setTimeout(1000);
        window->start();
    );
}

TEST_CASE("SurfaceRenderer on stream of surfaces", "[fast][MeshRenderer][visual]") {
    CHECK_NOTHROW(
        ImageFileStreamer::pointer mhdStreamer = ImageFileStreamer::New();
        mhdStreamer->setFilenameFormat(std::string(FAST_TEST_DATA_DIR)+"US-3Dt/US-3Dt_#.mhd");
        SurfaceExtraction::pointer extractor = SurfaceExtraction::New();
        extractor->setInput(mhdStreamer->getOutput());
        extractor->setThreshold(200);
        MeshRenderer::pointer renderer = MeshRenderer::New();
        renderer->setInput(extractor->getOutput());
        SimpleWindow::pointer window = SimpleWindow::New();
        window->addRenderer(renderer);
        window->setTimeout(1000);
        window->start();
    );
}
