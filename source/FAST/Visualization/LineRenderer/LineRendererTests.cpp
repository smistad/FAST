#include <FAST/Importers/ImageFileImporter.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include "FAST/Testing.hpp"
#include "FAST/Importers/VTKMeshFileImporter.hpp"
#include "LineRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"

using namespace fast;

TEST_CASE("LineRenderer", "[fast][LineRenderer][visual]") {
    VTKMeshFileImporter::pointer importer = VTKMeshFileImporter::New();
    importer->setFilename(Config::getTestDataPath() + "centerline.vtk");

    LineRenderer::pointer renderer = LineRenderer::New();
    renderer->addInputConnection(importer->getOutputPort());
    renderer->setColor(0, Color::Red());
    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->setTimeout(500);
    window->start();
}

TEST_CASE("LineRenderer 2D", "[fast][LineRenderer][visual]") {

    auto importer = ImageFileImporter::New();
    importer->setFilename(Config::getTestDataPath()+"US/CarotidArtery/Right/US-2D_0.mhd");
    auto imageRenderer = ImageRenderer::New();
    imageRenderer->addInputConnection(importer->getOutputPort());

    std::vector<MeshVertex> vertices = {
            MeshVertex(Vector3f(1, 5, 0)),
            MeshVertex(Vector3f(10, 10, 0)),
            MeshVertex(Vector3f(20, 20, 0)),
    };
    std::vector<MeshLine> lines = {
            MeshLine(0, 1),
            MeshLine(1, 2),
    };
    auto mesh = Mesh::create(vertices, lines);

    auto renderer = LineRenderer::New();
    renderer->addInputData(mesh);
    renderer->setDefaultLineWidth(0.5);
    renderer->setColor(0, Color::Red());

    auto window = SimpleWindow::New();
    window->addRenderer(imageRenderer);
    window->addRenderer(renderer);
    window->setTimeout(500);
    window->set2DMode();
    window->start();
}
