#include <FAST/Testing.hpp>
#include <FAST/Importers/WholeSlideImageImporter.hpp>
#include <FAST/Visualization/VeryLargeImageRenderer/VeryLargeImageRenderer.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Data/WholeSlideImage.hpp>
#include <FAST/Data/Image.hpp>
#include <FAST/Algorithms/ImagePatch/PatchGenerator.hpp>
#include <FAST/Algorithms/ImagePatch/PatchStitcher.hpp>
#include <FAST/Algorithms/ImagePatch/ImageToBatchGenerator.hpp>
#include <FAST/Algorithms/NeuralNetwork/NeuralNetwork.hpp>

using namespace fast;

TEST_CASE("Patch generator", "[fast][wsi][PatchGenerator]") {
    auto importer = WholeSlideImageImporter::New();
    importer->setFilename(Config::getTestDataPath() + "/CMU-1.tiff");

    auto generator = PatchGenerator::New();
    generator->setPatchSize(512, 512);
    generator->setPatchLevel(0);
    generator->setInputConnection(importer->getOutputPort());

    auto renderer = ImageRenderer::New();
    renderer->addInputConnection(generator->getOutputPort());
    auto window = SimpleWindow::New();
    window->addRenderer(renderer);
    //window->setTimeout(1000);
    window->set2DMode();
    window->start();
}

TEST_CASE("Patch generator and sticher", "[fast][wsi][PatchStitcher]") {
    auto importer = WholeSlideImageImporter::New();
    importer->setFilename(Config::getTestDataPath() + "/CMU-1.tiff");

    auto generator = PatchGenerator::New();
    generator->setPatchSize(512, 512);
    generator->setPatchLevel(4);
    generator->setInputConnection(importer->getOutputPort());

    auto stitcher = PatchStitcher::New();
    stitcher->setInputConnection(generator->getOutputPort());

    auto renderer = ImageRenderer::New();
    renderer->addInputConnection(stitcher->getOutputPort());
    auto window = SimpleWindow::New();
    window->addRenderer(renderer);
    //window->setTimeout(1000);
    window->set2DMode();
    window->start();
}

TEST_CASE("Patch generator, sticher and image to batch generator", "[fast][wsi][ImageToBatchGenerator]") {
    auto importer = WholeSlideImageImporter::New();
    importer->setFilename(Config::getTestDataPath() + "/CMU-1.tiff");

    auto generator = PatchGenerator::New();
    generator->setPatchSize(512, 512);
    generator->setPatchLevel(4);
    generator->setInputConnection(importer->getOutputPort());

    auto batchGenerator = ImageToBatchGenerator::New();
    batchGenerator->setInputConnection(generator->getOutputPort());
    batchGenerator->setMaxBatchSize(4);
    auto port = batchGenerator->getOutputPort();

    for(int i = 0; i < 12; ++i) {
        batchGenerator->update(i); // This will only call execute once
    }
    CHECK(port->getSize() == 3); // Should have 3 batches
    auto batch = port->getNextFrame<Batch>(); // This will block if batch does not exist at current timestep (11)
}