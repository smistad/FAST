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
    auto importer = WholeSlideImageImporter::create(Config::getTestDataPath() + "/WSI/CMU-1.svs");

    auto segmentation = TissueSegmentation::create()
            ->connect(importer);

    auto renderer = ImagePyramidRenderer::create()
            ->connect(importer);

    auto segRenderer = SegmentationRenderer::create()
            ->connect(segmentation);
    segRenderer->setOpacity(0.5);

    auto window = SimpleWindow2D::create();
    window->addRenderer(renderer);
    window->addRenderer(segRenderer);
    window->setTimeout(2000);
    window->start();
}

TEST_CASE("Tissue segmentation on image", "[fast][wsi][TissueSegmentation][visual]") {
    auto importer = WholeSlideImageImporter::create(Config::getTestDataPath() + "/WSI/CMU-1.svs");

    auto access = importer->runAndGetOutputData<ImagePyramid>()->getAccess(ACCESS_READ);
    auto image = access->getLevelAsImage(2);

    auto segmentation = TissueSegmentation::create()
            ->connect(image);

    auto renderer = ImagePyramidRenderer::create()
            ->connect(importer);

    auto segRenderer = SegmentationRenderer::create()
            ->connect(segmentation);
    segRenderer->setOpacity(0.5);

    auto window = SimpleWindow2D::create();
    window->addRenderer(renderer);
    window->addRenderer(segRenderer);
    window->setTimeout(2000);
    window->start();
}