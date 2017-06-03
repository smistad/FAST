#include "FAST/Testing.hpp"
#include "TubeSegmentationAndCenterlineExtraction.hpp"
#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/Visualization/SliceRenderer/SliceRenderer.hpp"
#include "FAST/Visualization/LineRenderer/LineRenderer.hpp"
#include "FAST/Algorithms/SurfaceExtraction/SurfaceExtraction.hpp"
#include "FAST/Visualization/TriangleRenderer/TriangleRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"
#include "FAST/Algorithms/ImageCropper/ImageCropper.hpp"

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

    TriangleRenderer::pointer TriangleRenderer = TriangleRenderer::New();
    TriangleRenderer->addInputConnection(surfaceExtraction->getOutputPort());

    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->addRenderer(TriangleRenderer);
    window->addRenderer(lineRenderer);
    window->start();
}
*/

/*
TEST_CASE("TSF Airway", "[tsf][airway][visual][broken_on_mac]") {
    Reporter::setGlobalReportMethod(Reporter::COUT);
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(Config::getTestDataPath() + "/CT/CT-Thorax.mhd");
    //importer->setFilename("/home/smistad/Data/lunge_datasett/pasient17.mhd");

    // Need to know the data type
    importer->update();
    Image::pointer image = importer->getOutputData<Image>();

    TubeSegmentationAndCenterlineExtraction::pointer tubeExtraction = TubeSegmentationAndCenterlineExtraction::New();
    tubeExtraction->setInputConnection(importer->getOutputPort());
    tubeExtraction->extractDarkTubes();
    tubeExtraction->enableAutomaticCropping(true);
    // Set min and max intensity based on HU unit scale
    if(image->getDataType() == TYPE_UINT16) {
        tubeExtraction->setMinimumIntensity(0);
        tubeExtraction->setMaximumIntensity(1124);
    } else {
        tubeExtraction->setMinimumIntensity(-1024);
        tubeExtraction->setMaximumIntensity(100);
    }
    tubeExtraction->setMinimumRadius(0.5);
    tubeExtraction->setMaximumRadius(50);
    tubeExtraction->setSensitivity(0.8);
    tubeExtraction->enableRuntimeMeasurements();

    SliceRenderer::pointer renderer = SliceRenderer::New();
    renderer->setInputConnection(importer->getOutputPort());

    LineRenderer::pointer lineRenderer = LineRenderer::New();
    lineRenderer->addInputConnection(tubeExtraction->getCenterlineOutputPort());
    lineRenderer->setDefaultDrawOnTop(true);

    SurfaceExtraction::pointer surfaceExtraction = SurfaceExtraction::New();
    surfaceExtraction->setInputConnection(tubeExtraction->getSegmentationOutputPort());

    TriangleRenderer::pointer TriangleRenderer = TriangleRenderer::New();
    TriangleRenderer->addInputConnection(surfaceExtraction->getOutputPort());

    SimpleWindow::pointer window = SimpleWindow::New();
    //window->addRenderer(renderer);
    window->addRenderer(TriangleRenderer);
    window->addRenderer(lineRenderer);
    window->setTimeout(3*1000);
    window->start();
    tubeExtraction->getRuntime()->print();
}
 */

}
