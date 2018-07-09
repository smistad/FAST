#include "FAST/Testing.hpp"
#include "FAST/Importers/DICOMFileImporter.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SliceRenderer/SliceRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"

using namespace fast;

TEST_CASE("Dicom image read", "[DICOM][visual]") {
    DICOMFileImporter::pointer importer = DICOMFileImporter::New();
    importer->setLoadSeries(false);
    importer->setFilename("/home/smistad/data/lungdb/W0031/1.2.826.0.1.3680043.2.656.1.76/S02A01/1.2.826.0.1.3680043.2.656.1.78.1.dcm");

    ImageRenderer::pointer renderer = ImageRenderer::New();
    renderer->setInputConnection(importer->getOutputPort());
    renderer->setIntensityLevel(0);
    renderer->setIntensityWindow(2048);
    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->setSize(1024, 512);
    window->set2DMode();
    window->setTimeout(500);

    CHECK_NOTHROW(window->start());

}

TEST_CASE("Dicom image read 3D", "[DICOM][visual]") {
    DICOMFileImporter::pointer importer = DICOMFileImporter::New();
    importer->setLoadSeries(true);
    importer->setFilename("/home/smistad/data/lungdb/W0031/1.2.826.0.1.3680043.2.656.1.76/S02A01/1.2.826.0.1.3680043.2.656.1.78.1.dcm");

    SliceRenderer::pointer renderer = SliceRenderer::New();
    renderer->setInputConnection(importer->getOutputPort());
    renderer->setIntensityLevel(0);
    renderer->setIntensityWindow(2048);
    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->setSize(1024, 512);
    window->set2DMode();
    window->setTimeout(500);

    CHECK_NOTHROW(window->start());

}
