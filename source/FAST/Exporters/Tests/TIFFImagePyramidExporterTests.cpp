#include <FAST/Exporters/TIFFImagePyramidExporter.hpp>
#include <FAST/Testing.hpp>
#include <FAST/Importers/WholeSlideImageImporter.hpp>

#include <FAST/Visualization/ImagePyramidRenderer/ImagePyramidRenderer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Importers/ImagePyramidPatchImporter.hpp>
#include <FAST/Visualization/SegmentationRenderer/SegmentationRenderer.hpp>
#include <FAST/Algorithms/ImagePatch/PatchGenerator.hpp>
#include <FAST/Algorithms/BinaryThresholding/BinaryThresholding.hpp>
#include <FAST/Algorithms/ImagePatch/PatchStitcher.hpp>

using namespace fast;

/*
TEST_CASE("TIFFImagePyramidExporter", "[fast][TIFFImagePyramidExporter]") {
    auto importer = WholeSlideImageImporter::New();
    importer->setFilename("/home/smistad/Downloads/OS-1.tiff");

    auto exporter = TIFFImagePyramidExporter::New();
    exporter->setFilename("image-pyramid-test.tiff");
    exporter->setInputConnection(importer->getOutputPort());
    exporter->enableRuntimeMeasurements();
    exporter->update();

    auto importer2 = WholeSlideImageImporter::New();
    importer2->setFilename("image-pyramid-test.tiff");

    auto renderer = ImagePyramidRenderer::New();
    renderer->setInputConnection(importer2->getOutputPort());

    auto window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->set2DMode();
    window->start();
    exporter->getAllRuntimes()->printAll();
}

TEST_CASE("TIFFImagePyramidExporter segmentation2", "[fast][TIFFImagePyramidExporter]") {
    auto importer = WholeSlideImageImporter::New();
    importer->setFilename("/home/smistad/Downloads/OS-1.tiff");

    auto generator = PatchGenerator::New();
    generator->setInputConnection(importer->getOutputPort());
    generator->setPatchLevel(3);
    generator->setPatchSize(512, 512);

    auto segmentation = BinaryThresholding::New();
    segmentation->setInputConnection(generator->getOutputPort());
    segmentation->setUpperThreshold(100);
    segmentation->setLowerThreshold(0);


    auto stitcher = PatchStitcher::New();
    stitcher->setInputConnection(segmentation->getOutputPort());
    auto port = stitcher->getOutputPort();

    auto renderer = SegmentationRenderer::New();
    renderer->setInputConnection(stitcher->getOutputPort());

    auto renderer2 = ImagePyramidRenderer::New();
    renderer2->addInputConnection(importer->getOutputPort());

    auto window = SimpleWindow::New();
    window->addRenderer(renderer2);
    window->addRenderer(renderer);
    window->set2DMode();
    window->start();

    auto exporter = TIFFImagePyramidExporter::New();
    exporter->setFilename("image-pyramid-segmentation-test2.tiff");
    exporter->setInputData(port->getNextFrame());
    exporter->enableRuntimeMeasurements();
    exporter->update();
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
*/