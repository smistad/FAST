#include "catch.hpp"
#include "VTKLineSetFileImporter.hpp"
#include "LineRenderer.hpp"
#include "SimpleWindow.hpp"

using namespace fast;

TEST_CASE("LineRenderer", "[fast][LineRenderer][visual]") {
    VTKLineSetFileImporter::pointer importer = VTKLineSetFileImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR) + "centerline.vtk");

    LineRenderer::pointer renderer = LineRenderer::New();
    renderer->addInputConnection(importer->getOutputPort());
    renderer->setColor(importer->getOutputPort(), Color::Red());
    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->setTimeout(500);
    window->start();
}
