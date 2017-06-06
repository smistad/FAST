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
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(Config::getTestDataPath() + "CT/CT-Abdomen.mhd");

    LevelSetSegmentation::pointer segmentation = LevelSetSegmentation::New();
    segmentation->setIntensityMean(150);
    segmentation->setIntensityVariance(50);
    segmentation->setCurvatureWeight(0.75);
    segmentation->setMaxIterations(5000);
    segmentation->addSeedPoint(Vector3i(149, 210, 345), 10);
    segmentation->setInputConnection(importer->getOutputPort());
    segmentation->update();

    SurfaceExtraction::pointer extraction = SurfaceExtraction::New();
    extraction->setInputConnection(segmentation->getOutputPort());

    TriangleRenderer::pointer renderer = TriangleRenderer::New();
    renderer->addInputConnection(extraction->getOutputPort());

    SegmentationRenderer::pointer segRenderer = SegmentationRenderer::New();
    segRenderer->addInputConnection(segmentation->getOutputPort());

    ImageRenderer::pointer imageRenderer = ImageRenderer::New();
    imageRenderer->addInputConnection(importer->getOutputPort());

    DualViewWindow::pointer window = DualViewWindow::New();
    window->getTopLeftView()->addRenderer(renderer);
    window->getTopLeftView()->set3DMode();
    window->getBottomRightView()->addRenderer(imageRenderer);
    window->getBottomRightView()->addRenderer(segRenderer);
    window->getBottomRightView()->set2DMode();
    window->start();
}
    */
