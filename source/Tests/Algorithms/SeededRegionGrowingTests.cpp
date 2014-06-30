#include "catch.hpp"
#include "SeededRegionGrowing.hpp"
#include "ImageImporter.hpp"
#include "ImageRenderer.hpp"
#include "SimpleWindow.hpp"
#include "DeviceManager.hpp"

namespace fast {

TEST_CASE("2D Seeded region growing on OpenCL device", "[fast][SeededRegionGrowing]") {
    ImageImporter::pointer importer = ImageImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR) + "lena.jpg");
    
    SeededRegionGrowing::pointer algorithm = SeededRegionGrowing::New();
    algorithm->setInput(importer->getOutput());
    algorithm->addSeedPoint(100,100);
    algorithm->setIntensityRange(0.4,0.6);
    Image::pointer result = algorithm->getOutput();
    
    ImageRenderer::pointer renderer = ImageRenderer::New();
    renderer->setInput(result);
    renderer->setIntensityLevel(0.5);
    renderer->setIntensityWindow(1);
    
    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->runMainLoop();
    
}

} // end namespace fast
