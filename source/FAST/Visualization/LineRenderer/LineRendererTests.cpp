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
    renderer->setColor(importer->getOutputPort(), Color::Red());
    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->setTimeout(500);
    window->start();
}
