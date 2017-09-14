#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Importers/VTKMeshFileImporter.hpp>
#include "FAST/Testing.hpp"
#include "VertexRenderer.hpp"

namespace fast {

TEST_CASE("VertexRenderer on LV surface model", "[fast][VertexRenderer][visual]") {
    CHECK_NOTHROW(
            VTKMeshFileImporter::pointer importer = VTKMeshFileImporter::New();
            importer->setFilename(Config::getTestDataPath() + "Surface_LV.vtk");
            VertexRenderer::pointer renderer = VertexRenderer::New();
            renderer->addInputConnection(importer->getOutputPort());
            SimpleWindow::pointer window = SimpleWindow::New();
            window->addRenderer(renderer);
            //window->setTimeout(1000);
            window->start();
    );
}

}
