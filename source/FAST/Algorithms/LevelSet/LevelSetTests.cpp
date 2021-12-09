#include <FAST/Visualization/SliceRenderer/SliceRenderer.hpp>
#include "FAST/Testing.hpp"
#include "LevelSetSegmentation.hpp"
#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/Algorithms/SurfaceExtraction/SurfaceExtraction.hpp"
#include "FAST/Visualization/TriangleRenderer/TriangleRenderer.hpp"
#include "FAST/Visualization/SegmentationRenderer/SegmentationRenderer.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"
#include "FAST/Visualization/DualViewWindow.hpp"

using namespace fast;

/*
TEST_CASE("Level set segmentation", "[fast][levelset][visual]") {
    auto importer = ImageFileImporter::create(Config::getTestDataPath() + "CT/CT-Abdomen.mhd");

    auto segmentation = LevelSetSegmentation::New();
    segmentation->setIntensityMean(150);
    segmentation->setIntensityVariance(50);
    segmentation->setCurvatureWeight(0.5);
    segmentation->setMaxIterations(500);
    segmentation->addSeedPoint(Vector3i(149, 210, 345), 10);
    segmentation->setInputConnection(importer->getOutputPort());

    auto extraction = SurfaceExtraction::create()
            ->connect(segmentation);

    auto sliceRenderer = SliceRenderer::create(fast::PLANE_Z, 345)
            ->connect(importer);

    auto renderer = TriangleRenderer::create();
    renderer->addInputConnection(extraction->getOutputPort());

    auto window = DualViewWindow::create();
    window->getTopLeftView()->addRenderer(sliceRenderer);
    window->getTopLeftView()->addRenderer(renderer);
    window->getTopLeftView()->set3DMode();
    window->start();
}
*/