#include <FAST/Exporters/TIFFImagePyramidExporter.hpp>
#include <FAST/Testing.hpp>
#include <FAST/Importers/WholeSlideImageImporter.hpp>

#include <FAST/Visualization/ImagePyramidRenderer/ImagePyramidRenderer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Importers/ImagePyramidPatchImporter.hpp>
#include <FAST/Visualization/SegmentationRenderer/SegmentationRenderer.hpp>

using namespace fast;

TEST_CASE("TIFFImagePyramidExporter", "[fast][TIFFImagePyramidExporter]") {
    auto importer = WholeSlideImageImporter::New();
    importer->setFilename(Config::getTestDataPath() + "/WSI/A05.svs");

    auto exporter = TIFFImagePyramidExporter::New();
    exporter->setFilename("image-pyramid-test.tiff");
    exporter->setInputConnection(importer->getOutputPort());
    exporter->enableRuntimeMeasurements();
    exporter->update();

    importer = WholeSlideImageImporter::New();
    importer->setFilename("image-pyramid-test.tiff");

    auto renderer = ImagePyramidRenderer::New();
    renderer->setInputConnection(importer->getOutputPort());

    auto window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->set2DMode();
    window->start();
    exporter->getAllRuntimes()->printAll();
}

TEST_CASE("TIFFImagePyramidExporter segmentation", "[fast][TIFFImagePyramidExporter]") {
    auto importer = ImagePyramidPatchImporter::New();
    importer->setPath("/home/smistad/test_image_pyramid_export_level_0/");

    auto exporter = TIFFImagePyramidExporter::New();
    exporter->setFilename("image-pyramid-segmentation-test.tiff");
    exporter->setInputConnection(importer->getOutputPort());
    exporter->enableRuntimeMeasurements();
    exporter->update();

    auto importer2 = WholeSlideImageImporter::New();
    importer2->setFilename("image-pyramid-segmentation-test.tiff");

    auto renderer = SegmentationRenderer::New();
    renderer->setInputConnection(importer2->getOutputPort());

    auto window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->set2DMode();
    window->start();
    exporter->getAllRuntimes()->printAll();
}
