#include <FAST/Testing.hpp>
#include "CenterlineExtraction.hpp"
#include <FAST/Importers/ImageFileImporter.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Visualization/LineRenderer/LineRenderer.hpp>
#include <FAST/Visualization/SegmentationRenderer/SegmentationRenderer.hpp>

using namespace fast;

/*
TEST_CASE("Centerline extraction 2D", "[fast][CenterlineExtraction][2D]") {
    auto importer = ImageFileImporter::New();
    importer->setFilename(Config::getTestDataPath() + "retina.png");

    auto centerline = CenterlineExtraction::New();
    centerline->enableRuntimeMeasurements();
    centerline->setInputConnection(importer->getOutputPort());

    auto lineRenderer = LineRenderer::New();
    lineRenderer->addInputConnection(centerline->getOutputPort());

    auto segRenderer = SegmentationRenderer::New();
    segRenderer->addInputConnection(importer->getOutputPort());

    auto window = SimpleWindow::New();
    window->set2DMode();
    window->addRenderer(segRenderer);
    window->addRenderer(lineRenderer);
    window->start();
    centerline->getRuntime()->print();
}
 */
