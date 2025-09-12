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

    auto importer = ImageFileImporter::create(Config::getTestDataPath()+"US/CarotidArtery/Right/US-2D_0.mhd");
    auto imageRenderer = ImageRenderer::create()->connect(importer);

    std::vector<MeshVertex> vertices = {
            MeshVertex(Vector3f(1, 5, 0)),
            MeshVertex(Vector3f(10, 10, 0)),
            MeshVertex(Vector3f(20, 20, 0)),
    };
    auto mesh = Mesh::create(vertices);

    auto renderer = VertexRenderer::create(10, false, 1)->connect(mesh);
    renderer->setOpacity(0.5);

    auto window = SimpleWindow2D::create()->connect(imageRenderer)->connect(renderer);
    window->setTimeout(500);
    window->run();
}

}
