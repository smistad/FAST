#include "catch.hpp"
#include "Skeletonization.hpp"
#include "ImageImporter.hpp"
#include "ImageRenderer.hpp"
#include "SimpleWindow.hpp"

using namespace fast;

TEST_CASE("Skeletonization on 2D image", "[fast][Skeletonization]") {
    ImageImporter::pointer importer = ImageImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR) + "retina.png");

    Skeletonization::pointer skeletonization = Skeletonization::New();
    skeletonization->setInput(importer->getOutput());
    skeletonization->enableRuntimeMeasurements();

    ImageRenderer::pointer renderer = ImageRenderer::New();
    renderer->addInput(skeletonization->getOutput());
    renderer->setIntensityWindow(1);
    renderer->setIntensityLevel(0.5);
    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->setTimeout(500);
    window->runMainLoop();
    skeletonization->getRuntime()->print();
}
