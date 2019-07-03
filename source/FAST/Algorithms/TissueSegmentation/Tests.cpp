#include <FAST/Testing.hpp>
#include <FAST/Importers/WholeSlideImageImporter.hpp>
#include <FAST/Visualization/VeryLargeImageRenderer/VeryLargeImageRenderer.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Visualization/SegmentationRenderer/SegmentationRenderer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Data/WholeSlideImage.hpp>
#include <FAST/Data/Image.hpp>
#include <FAST/Algorithms/TissueSegmentation/TissueSegmentation.hpp>

using namespace fast;

TEST_CASE("Tissue segmentation", "[fast][wsi][TissueSegmentation]") {
    auto importer = WholeSlideImageImporter::New();
    importer->setFilename(Config::getTestDataPath() + "/CMU-1.tiff");

    auto segmentation = TissueSegmentation::New();
    segmentation->setInputConnection(importer->getOutputPort());

    auto renderer = VeryLargeImageRenderer::New();
    renderer->addInputConnection(importer->getOutputPort());

    auto segRenderer = SegmentationRenderer::New();
    segRenderer->addInputConnection(segmentation->getOutputPort());
    segRenderer->setOpacity(0.5);

    auto window = SimpleWindow::New();
    window->set2DMode();
    window->addRenderer(renderer);
    window->addRenderer(segRenderer);
    window->start();
}