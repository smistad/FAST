#include <FAST/Importers/WholeSlideImageImporter.hpp>
#include <FAST/Testing.hpp>
#include <FAST/Visualization/VeryLargeImageRenderer/ImagePyramidRenderer.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Data/ImagePyramid.hpp>
#include <FAST/Data/Image.hpp>

using namespace fast;

TEST_CASE("Import whole slide image", "[fast][WholeSlideImageImporter][wsi]") {
    auto importer = WholeSlideImageImporter::New();
    importer->setFilename(Config::getTestDataPath() + "/CMU-1.tiff");

    auto renderer = ImagePyramidRenderer::New();
    renderer->addInputConnection(importer->getOutputPort());
    auto window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->set2DMode();
    window->setTimeout(1000);
    window->start();
}

TEST_CASE("Import whole slide image and display highest level", "[fast][WholeSlideImageImporter][wsi]") {
    auto importer = WholeSlideImageImporter::New();
    importer->setFilename(Config::getTestDataPath() + "/CMU-1.tiff");
    auto port = importer->getOutputPort();
    importer->update();
    auto WSI = port->getNextFrame<ImagePyramid>();
    auto image = WSI->getLevelAsImage(WSI->getNrOfLevels()-1);

    auto renderer = ImageRenderer::New();
    renderer->addInputData(image);
    auto window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->setTimeout(1000);
    window->set2DMode();
    window->start();
}


TEST_CASE("Import whole slide image and display a tile at level 0", "[fast][WholeSlideImageImporter][wsi]") {
    auto importer = WholeSlideImageImporter::New();
    importer->setFilename(Config::getTestDataPath() + "/CMU-1.tiff");
    auto port = importer->getOutputPort();
    importer->update();
    auto WSI = port->getNextFrame<ImagePyramid>();
    auto image = WSI->getPatchAsImage(0, 40000, 10000, 1024, 1024);

    auto renderer = ImageRenderer::New();
    renderer->addInputData(image);
    auto window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->setTimeout(1000);
    window->set2DMode();
    window->start();
}
