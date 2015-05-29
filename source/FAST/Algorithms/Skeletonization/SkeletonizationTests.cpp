#include "FAST/Tests/catch.hpp"
#include "Skeletonization.hpp"
#include "FAST/Importers/ImageImporter.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"
#include "FAST/Algorithms/BinaryThresholding/BinaryThresholding.hpp"

using namespace fast;

TEST_CASE("Skeletonization on 2D image", "[fast][Skeletonization][visual]") {
    ImageImporter::pointer importer = ImageImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR) + "retina.png");

    BinaryThresholding::pointer thresholding = BinaryThresholding::New();
    thresholding->setInputConnection(importer->getOutputPort());
    thresholding->setLowerThreshold(0.5);

    Skeletonization::pointer skeletonization = Skeletonization::New();
    skeletonization->setInputConnection(thresholding->getOutputPort());
    skeletonization->enableRuntimeMeasurements();

    ImageRenderer::pointer renderer = ImageRenderer::New();
    renderer->addInputConnection(skeletonization->getOutputPort());
    renderer->setIntensityWindow(1);
    renderer->setIntensityLevel(0.5);
    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->setTimeout(500);
    window->start();
    skeletonization->getRuntime()->print();
}
