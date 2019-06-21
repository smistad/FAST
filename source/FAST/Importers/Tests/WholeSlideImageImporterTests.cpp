#include <FAST/Importers/WholeSlideImageImporter.hpp>
#include <FAST/Testing.hpp>
#include <FAST/Visualization/VeryLargeImageRenderer/VeryLargeImageRenderer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>

using namespace fast;

TEST_CASE("Import whole slide image", "[fast][WholeSlideImageImporter][wsi]") {
    auto importer = WholeSlideImageImporter::New();
    importer->setFilename(Config::getTestDataPath() + "/CMU-1.tiff");

    auto renderer = VeryLargeImageRenderer::New();
    renderer->addInputConnection(importer->getOutputPort());
    auto window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->set2DMode();
    window->start();
}