#include <FAST/Testing.hpp>
#include <FAST/Importers/WholeSlideImageImporter.hpp>
#include <FAST/Visualization/ImagePyramidRenderer/ImagePyramidRenderer.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Visualization/SegmentationRenderer/SegmentationRenderer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Data/ImagePyramid.hpp>
#include <FAST/Data/Image.hpp>
#include <FAST/Algorithms/TissueSegmentation/TissueSegmentation.hpp>

using namespace fast;

TEST_CASE("Tissue segmentation", "[fast][wsi][TissueSegmentation][visual]") {
    auto importer = WholeSlideImageImporter::New();
    importer->setFilename(Config::getTestDataPath() + "/WSI/A05.svs");

    auto segmentation = TissueSegmentation::New();
    segmentation->setInputConnection(importer->getOutputPort());

    auto renderer = ImagePyramidRenderer::New();
    renderer->addInputConnection(importer->getOutputPort());

    auto segRenderer = SegmentationRenderer::New();
    segRenderer->addInputConnection(segmentation->getOutputPort());
    segRenderer->setOpacity(0.5);

    auto window = SimpleWindow::New();
    window->set2DMode();
    window->addRenderer(renderer);
    window->addRenderer(segRenderer);
    window->setTimeout(2000);
    window->start();
}