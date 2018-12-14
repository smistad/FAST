#include <FAST/Importers/WholeSlideImageImporter.hpp>
#include <FAST/Testing.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>

using namespace fast;

TEST_CASE("Import whole slide image", "[fast][WholeSlideImageImporter][wsi]") {
    auto importer = WholeSlideImageImporter::New();
    importer->setFilename("/home/smistad/Downloads/CMU-1.tiff");
    //importer->update(0);

    auto renderer = ImageRenderer::New();
    renderer->addInputConnection(importer->getOutputPort());
    auto window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->set2DMode();
    window->start();
}