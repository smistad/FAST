#include "FAST/Importers/VTKMeshFileImporter.hpp"
#include "FAST/Streamers/ImageFileStreamer.hpp"
#include "FAST/Tests/catch.hpp"
#include "MeshRenderer.hpp"
#include "FAST/Algorithms/SurfaceExtraction/SurfaceExtraction.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"

using namespace fast;

TEST_CASE("SurfaceRenderer on LV surface model", "[fast][MeshRenderer][visual]") {
    CHECK_NOTHROW(
        VTKMeshFileImporter::pointer importer = VTKMeshFileImporter::New();
        importer->setFilename(std::string(FAST_TEST_DATA_DIR) + "Surface_LV.vtk");
        MeshRenderer::pointer renderer = MeshRenderer::New();
        renderer->addInputConnection(importer->getOutputPort());
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
        extractor->setInputConnection(mhdStreamer->getOutputPort());
        extractor->setThreshold(200);
        MeshRenderer::pointer renderer = MeshRenderer::New();
        renderer->addInputConnection(extractor->getOutputPort());
        SimpleWindow::pointer window = SimpleWindow::New();
        window->addRenderer(renderer);
        window->setTimeout(1000);
        window->start();
    );
}
