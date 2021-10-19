#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Importers/VTKMeshFileImporter.hpp>
#include <FAST/Importers/ImageFileImporter.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include "FAST/Testing.hpp"
#include "VertexRenderer.hpp"

namespace fast {

TEST_CASE("VertexRenderer on LV surface model", "[fast][VertexRenderer][visual]") {
    CHECK_NOTHROW(
            auto importer = VTKMeshFileImporter::create(Config::getTestDataPath() + "Surface_LV.vtk");
            auto renderer = VertexRenderer::create(10.0f);
            renderer->connect(importer);
            auto window = SimpleWindow::create()->connect(renderer);
            window->setTimeout(1000);
            window->run();
    );
}

TEST_CASE("VertexRenderer 2D", "[fast][VertexRenderer][visual]") {

    auto importer = ImageFileImporter::New();
    importer->setFilename(Config::getTestDataPath()+"US/CarotidArtery/Right/US-2D_0.mhd");
    auto imageRenderer = ImageRenderer::New();
    imageRenderer->addInputConnection(importer->getOutputPort());

    std::vector<MeshVertex> vertices = {
            MeshVertex(Vector3f(1, 5, 0)),
            MeshVertex(Vector3f(10, 10, 0)),
            MeshVertex(Vector3f(20, 20, 0)),
    };
    auto mesh = Mesh::create(vertices);

    auto renderer = VertexRenderer::New();
    renderer->addInputData(mesh);
    renderer->setColor(0, Color::Red());

    auto window = SimpleWindow::New();
    window->addRenderer(imageRenderer);
    window->addRenderer(renderer);
    window->setTimeout(500);
    window->set2DMode();
    window->start();
}

}
