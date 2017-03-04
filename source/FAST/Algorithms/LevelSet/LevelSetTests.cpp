#include "FAST/Testing.hpp"
#include "LevelSetSegmentation.hpp"
#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/Algorithms/SurfaceExtraction/SurfaceExtraction.hpp"
#include "FAST/Visualization/MeshRenderer/MeshRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"

using namespace fast;

/*
TEST_CASE("Level set segmentation", "[fast][levelset][visual]") {
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(Config::getTestDataPath() + "CT/CT-Abdomen.mhd");

    LevelSetSegmentation::pointer segmentation = LevelSetSegmentation::New();
    segmentation->setIntensityMean(150);
    segmentation->setIntensityVariance(50);
    segmentation->setCurvatureWeight(0.99);
    segmentation->setMaxIterations(1000);
    segmentation->addSeedPoint(Vector3i(149, 210, 345), 10);
    segmentation->setInputConnection(importer->getOutputPort());
    segmentation->update();

    SurfaceExtraction::pointer extraction = SurfaceExtraction::New();
    extraction->setInputConnection(segmentation->getOutputPort());

    MeshRenderer::pointer renderer = MeshRenderer::New();
    renderer->addInputConnection(extraction->getOutputPort());

    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->set3DMode();
    window->start();
}
 */
