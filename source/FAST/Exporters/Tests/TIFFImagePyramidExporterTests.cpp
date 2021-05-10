#include <FAST/Exporters/TIFFImagePyramidExporter.hpp>
#include <FAST/Testing.hpp>
#include <FAST/Importers/WholeSlideImageImporter.hpp>
#include <FAST/Data/ImagePyramid.hpp>
#include <FAST/Visualization/ImagePyramidRenderer/ImagePyramidRenderer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Importers/ImagePyramidPatchImporter.hpp>
#include <FAST/Visualization/SegmentationRenderer/SegmentationRenderer.hpp>
#include <FAST/Algorithms/ImagePatch/PatchGenerator.hpp>
#include <FAST/Algorithms/BinaryThresholding/BinaryThresholding.hpp>
#include <FAST/Algorithms/ImagePatch/PatchStitcher.hpp>
#include <FAST/Importers/TIFFImagePyramidImporter.hpp>

using namespace fast;

TEST_CASE("TIFFImagePyramidExporter", "[fast][TIFFImagePyramidExporter][wsi][visual]") {
    auto importer = WholeSlideImageImporter::New();
    importer->setFilename(Config::getTestDataPath() + "/WSI/A05.svs");

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
    window->setTimeout(2000);
    window->start();
    exporter->getAllRuntimes()->printAll();
}

TEST_CASE("TIFFImagePyramidExporter segmentation2", "[fast][TIFFImagePyramidExporter][wsi][visual]") {
    auto importer = WholeSlideImageImporter::New();
    importer->setFilename(Config::getTestDataPath() + "/WSI/A05.svs");

    auto generator = PatchGenerator::New();
    generator->setInputConnection(importer->getOutputPort());
    generator->setPatchLevel(1);
    generator->setPatchSize(256, 256);
    generator->setOverlap(0.15);

    auto segmentation = BinaryThresholding::New();
    segmentation->setInputConnection(generator->getOutputPort());
    segmentation->setLowerThreshold(200);

    auto stitcher = PatchStitcher::New();
    stitcher->setInputConnection(segmentation->getOutputPort());
    stitcher->enableRuntimeMeasurements();
    auto port = stitcher->getOutputPort();

    {
        auto renderer1 = ImagePyramidRenderer::New();
        renderer1->setInputConnection(importer->getOutputPort());

        auto renderer = SegmentationRenderer::New();
        renderer->setInputConnection(stitcher->getOutputPort());

        auto window = SimpleWindow::New();
        window->addRenderer(renderer1);
        window->addRenderer(renderer);
        window->set2DMode();
        window->start();
    }
    // Wait until finished..
    ImagePyramid::pointer image;
    do {
        stitcher->update();
        image = port->getNextFrame<ImagePyramid>();
    } while(!image->isLastFrame());

    // Export
    auto result = port->getNextFrame<ImagePyramid>();
    auto exporter = TIFFImagePyramidExporter::New();
    exporter->setFilename("image-pyramid-segmentation-test2.tiff");
    exporter->setInputData(result);
    exporter->enableRuntimeMeasurements();
    exporter->update();

    // Then import again and visualize
    auto importer2 = TIFFImagePyramidImporter::New();
    importer2->setFilename("image-pyramid-segmentation-test2.tiff");

    auto renderer = SegmentationRenderer::New();
    renderer->setInputConnection(importer2->getOutputPort());

    auto window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->set2DMode();
    //window->setTimeout(5000);
    window->start();
    //exporter->getAllRuntimes()->printAll();
}

/*
TEST_CASE("TIFFImagePyramidExporter segmentation", "[fast][TIFFImagePyramidExporter]") {
    auto importer = ImagePyramidPatchImporter::New();
    importer->setPath("/home/smistad/test_image_pyramid_export_level_0/");

    auto exporter = TIFFImagePyramidExporter::New();
    exporter->setFilename("image-pyramid-segmentation-test.tiff");
    exporter->setInputConnection(importer->getOutputPort());
    exporter->enableRuntimeMeasurements();
    exporter->update();

    auto importer2 = TIFFImagePyramidImporter::New();
    importer2->setFilename("image-pyramid-segmentation-test.tiff");

    auto renderer = SegmentationRenderer::New();
    renderer->setInputConnection(importer2->getOutputPort());

    auto window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->set2DMode();
    window->start();
    exporter->getAllRuntimes()->printAll();
}*/