#include "FAST/Testing.hpp"
#include "FAST/Importers/DICOMFileImporter.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SliceRenderer/SliceRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"

using namespace fast;

TEST_CASE("Dicom image read", "[DICOM][DICOMFileImporter][visual]") {
    auto importer = DICOMFileImporter::New();
    importer->setLoadSeries(false);
    importer->setFilename(Config::getTestDataPath() + "/CT/LIDC-IDRI-0072/000001.dcm");

    auto renderer = ImageRenderer::New();
    renderer->setInputConnection(importer->getOutputPort());
    renderer->setIntensityLevel(0);
    renderer->setIntensityWindow(2048);
    auto window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->setSize(1024, 512);
    window->set2DMode();
    window->setTimeout(500);

    CHECK_NOTHROW(window->start());

}

TEST_CASE("Dicom image read 3D", "[DICOM][visual]") {
    auto importer = DICOMFileImporter::New();
    importer->setLoadSeries(true);
    importer->setFilename(Config::getTestDataPath() + "/CT/LIDC-IDRI-0072/000001.dcm");

    auto renderer = SliceRenderer::New();
    renderer->setInputConnection(importer->getOutputPort());
    renderer->setIntensityLevel(0);
    renderer->setIntensityWindow(2048);
    auto window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->setSize(1024, 512);
    window->set2DMode();
    window->setTimeout(500);

    CHECK_NOTHROW(window->start());

}
