#include "FAST/Tests/catch.hpp"
#include "TubeSegmentationAndCenterlineExtraction.hpp"
#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/Visualization/SliceRenderer/SliceRenderer.hpp"
#include "FAST/Visualization/LineRenderer/LineRenderer.hpp"
#include "FAST/Algorithms/SurfaceExtraction/SurfaceExtraction.hpp"
#include "FAST/Visualization/MeshRenderer/MeshRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"

namespace fast {

/*
TEST_CASE("TSF", "[tsf]") {
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename("/home/smistad/Dropbox/Programmering/Tube-Segmentation-Framework/tests/data/synthetic/dataset_1/noisy.mhd");

    TubeSegmentationAndCenterlineExtraction::pointer tubeExtraction = TubeSegmentationAndCenterlineExtraction::New();
    tubeExtraction->setInputConnection(importer->getOutputPort());
    tubeExtraction->extractBrightTubes();
    tubeExtraction->setMinimumRadius(0.5);
    tubeExtraction->setMaximumRadius(8);
    tubeExtraction->setSensitivity(0.99);

    SliceRenderer::pointer renderer = SliceRenderer::New();
    renderer->setInputConnection(tubeExtraction->getTDFOutputPort());

    LineRenderer::pointer lineRenderer = LineRenderer::New();
    lineRenderer->addInputConnection(tubeExtraction->getCenterlineOutputPort());
    lineRenderer->setDefaultDrawOnTop(true);

    SurfaceExtraction::pointer surfaceExtraction = SurfaceExtraction::New();
    surfaceExtraction->setInputConnection(tubeExtraction->getSegmentationOutputPort());

    MeshRenderer::pointer meshRenderer = MeshRenderer::New();
    meshRenderer->addInputConnection(surfaceExtraction->getOutputPort());

    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->addRenderer(meshRenderer);
    window->addRenderer(lineRenderer);
    window->start();
}

TEST_CASE("TSF Airway", "[tsf][airway]") {
    Report::setReportMethod(Report::COUT);
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    //importer->setFilename(std::string(FAST_TEST_DATA_DIR) + "CT-Thorax.mhd");
    importer->setFilename("/home/smistad/Dropbox/CT-Thorax-cropped.mhd");

    TubeSegmentationAndCenterlineExtraction::pointer tubeExtraction = TubeSegmentationAndCenterlineExtraction::New();
    tubeExtraction->setInputConnection(importer->getOutputPort());
    tubeExtraction->extractDarkTubes();
    tubeExtraction->setMinimumIntensity(-1024);
    tubeExtraction->setMaximumIntensity(100);
    tubeExtraction->setMinimumRadius(3);
    tubeExtraction->setMaximumRadius(30);
    tubeExtraction->setSensitivity(0.85);

    SliceRenderer::pointer renderer = SliceRenderer::New();
    renderer->setInputConnection(tubeExtraction->getTDFOutputPort());

    LineRenderer::pointer lineRenderer = LineRenderer::New();
    lineRenderer->addInputConnection(tubeExtraction->getCenterlineOutputPort());
    lineRenderer->setDefaultDrawOnTop(true);

    SurfaceExtraction::pointer surfaceExtraction = SurfaceExtraction::New();
    surfaceExtraction->setInputConnection(tubeExtraction->getSegmentationOutputPort());

    MeshRenderer::pointer meshRenderer = MeshRenderer::New();
    meshRenderer->addInputConnection(surfaceExtraction->getOutputPort());

    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->addRenderer(meshRenderer);
    window->addRenderer(lineRenderer);
    window->start();
}
*/

}
