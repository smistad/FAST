#include "FAST/Importers/VTKMeshFileImporter.hpp"
#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/Streamers/ImageFileStreamer.hpp"
#include "FAST/Testing.hpp"
#include "TriangleRenderer.hpp"
#include "FAST/Algorithms/SurfaceExtraction/SurfaceExtraction.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Data/Mesh.hpp"

using namespace fast;

TEST_CASE("TriangleRenderer on LV surface model", "[fast][TriangleRenderer][visual]") {
    CHECK_NOTHROW(
        VTKMeshFileImporter::pointer importer = VTKMeshFileImporter::New();
        importer->setFilename(Config::getTestDataPath() + "Surface_LV.vtk");
        TriangleRenderer::pointer renderer = TriangleRenderer::New();
        renderer->addInputConnection(importer->getOutputPort());
        SimpleWindow::pointer window = SimpleWindow::New();
        window->addRenderer(renderer);
        window->setTimeout(1000);
        window->start();
    );
}

TEST_CASE("TriangleRenderer on stream of surfaces", "[fast][TriangleRenderer][visual]") {
    CHECK_NOTHROW(
        ImageFileStreamer::pointer mhdStreamer = ImageFileStreamer::New();
        mhdStreamer->setFilenameFormat(Config::getTestDataPath()+"US/Ball/US-3Dt_#.mhd");
        SurfaceExtraction::pointer extractor = SurfaceExtraction::New();
        extractor->setInputConnection(mhdStreamer->getOutputPort());
        extractor->setThreshold(200);
        TriangleRenderer::pointer renderer = TriangleRenderer::New();
        renderer->addInputConnection(extractor->getOutputPort());
        SimpleWindow::pointer window = SimpleWindow::New();
        window->addRenderer(renderer);
        window->setTimeout(1000);
        window->start();
    );
}

TEST_CASE("TriangleRenderer with 2D mesh and image", "[fast][TriangleRenderer][visual]") {
    CHECK_NOTHROW(
        ImageFileImporter::pointer importer = ImageFileImporter::New();
        importer->setFilename(Config::getTestDataPath() + "US/CarotidArtery/Right/US-2D_0.mhd");
        ImageRenderer::pointer imageRenderer = ImageRenderer::New();
        imageRenderer->addInputConnection(importer->getOutputPort());

        // Create a simple 2D mesh with a single line
        Mesh::pointer mesh = Mesh::New();
        std::vector<MeshVertex> vertices;
        vertices.push_back(MeshVertex(Vector3f(0, 0, 0)));
        vertices.push_back(MeshVertex(Vector3f(10, 10, 0)));
        std::vector<MeshLine> lines;
        lines.push_back(MeshLine(0,1));
        mesh->create(vertices, lines);

        TriangleRenderer::pointer renderer = TriangleRenderer::New();
        renderer->setInputData(mesh);
        SimpleWindow::pointer window = SimpleWindow::New();
        window->set2DMode();
        window->addRenderer(imageRenderer);
        window->addRenderer(renderer);
        window->setTimeout(1000);
        window->start();
    );
}
