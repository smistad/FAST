#include "FAST/Importers/VTKMeshFileImporter.hpp"
#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/Streamers/ImageFileStreamer.hpp"
#include "FAST/Testing.hpp"
#include "MeshRenderer.hpp"
#include "FAST/Algorithms/SurfaceExtraction/SurfaceExtraction.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Data/Mesh.hpp"

using namespace fast;

TEST_CASE("MeshRenderer on LV surface model", "[fast][MeshRenderer][visual]") {
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

TEST_CASE("MeshRenderer on stream of surfaces", "[fast][MeshRenderer][visual]") {
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

TEST_CASE("MeshRenderer with 2D mesh and image", "[fast][MeshRenderer][visual]") {
    CHECK_NOTHROW(
        ImageFileImporter::pointer importer = ImageFileImporter::New();
        importer->setFilename(std::string(FAST_TEST_DATA_DIR) + "US-2Dt/US-2Dt_0.mhd");
        ImageRenderer::pointer imageRenderer = ImageRenderer::New();
        imageRenderer->addInputConnection(importer->getOutputPort());

        // Create a simple 2D mesh with a single line
        Mesh::pointer mesh = Mesh::New();
        Vector2f vertex0(0, 0);
        Vector2f vertex1(10, 10);
        std::vector<Vector2f> vertices;
        vertices.push_back(vertex0);
        vertices.push_back(vertex1);
        std::vector<VectorXui> lines;
        lines.push_back(Vector2ui(0,1));
        mesh->create(vertices, vertices, lines);

        MeshRenderer::pointer renderer = MeshRenderer::New();
        renderer->setInputData(mesh);
        SimpleWindow::pointer window = SimpleWindow::New();
        window->set2DMode();
        window->addRenderer(imageRenderer);
        window->addRenderer(renderer);
        window->setTimeout(1000);
        window->start();
    );
}
