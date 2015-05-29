#include "FAST/Tests/catch.hpp"
#include "SegmentationRenderer.hpp"
#include "FAST/Algorithms/BinaryThresholding/BinaryThresholding.hpp"
#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"
#include "FAST/Streamers/ImageFileStreamer.hpp"

using namespace fast;

TEST_CASE("SegmentationRenderer on a thresholded 2D image", "[fast][SegmentationRenderer][visual]") {
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR) + "US-2Dt/US-2Dt_0.mhd");

    BinaryThresholding::pointer segmentation = BinaryThresholding::New();
    segmentation->setInputConnection(importer->getOutputPort());
    segmentation->setLowerThreshold(100);

    ImageRenderer::pointer imageRenderer = ImageRenderer::New();
    imageRenderer->addInputConnection(importer->getOutputPort());

    SegmentationRenderer::pointer renderer = SegmentationRenderer::New();
    renderer->addInputConnection(segmentation->getOutputPort());

    SimpleWindow::pointer window = SimpleWindow::New();
    window->set2DMode();
    window->addRenderer(imageRenderer);
    window->addRenderer(renderer);
    window->setTimeout(1000);
    CHECK_NOTHROW(window->start());
}

TEST_CASE("SegmentationRenderer on a thresholded 3D image", "[fast][SegmentationRenderer][visual]") {
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR) + "CT-Abdomen.mhd");

    BinaryThresholding::pointer segmentation = BinaryThresholding::New();
    segmentation->setInputConnection(importer->getOutputPort());
    segmentation->setLowerThreshold(100);

    ImageRenderer::pointer imageRenderer = ImageRenderer::New();
    imageRenderer->addInputConnection(importer->getOutputPort());

    SegmentationRenderer::pointer renderer = SegmentationRenderer::New();
    renderer->addInputConnection(segmentation->getOutputPort());

    SimpleWindow::pointer window = SimpleWindow::New();
    window->set2DMode();
    window->addRenderer(imageRenderer);
    window->addRenderer(renderer);
    window->setTimeout(1000);
    CHECK_NOTHROW(window->start());
}


TEST_CASE("SegmentationRenderer on a thresholded 3D image with draw contours only", "[fast][SegmentationRenderer][visual]") {
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR) + "CT-Abdomen.mhd");

    BinaryThresholding::pointer segmentation = BinaryThresholding::New();
    segmentation->setInputConnection(importer->getOutputPort());
    segmentation->setLowerThreshold(100);

    ImageRenderer::pointer imageRenderer = ImageRenderer::New();
    imageRenderer->addInputConnection(importer->getOutputPort());

    SegmentationRenderer::pointer renderer = SegmentationRenderer::New();
    renderer->setFillArea(false);
    renderer->addInputConnection(segmentation->getOutputPort());

    SimpleWindow::pointer window = SimpleWindow::New();
    window->set2DMode();
    window->addRenderer(imageRenderer);
    window->addRenderer(renderer);
    window->setTimeout(1000);
    CHECK_NOTHROW(window->start());
}


TEST_CASE("SegmentationRenderer on a stream of thresholded 2D images", "[fast][SegmentationRenderer][visual]") {
    ImageFileStreamer::pointer importer = ImageFileStreamer::New();
    importer->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-2Dt/US-2Dt_#.mhd");

    BinaryThresholding::pointer segmentation = BinaryThresholding::New();
    segmentation->setInputConnection(importer->getOutputPort());
    segmentation->setLowerThreshold(100);

    ImageRenderer::pointer imageRenderer = ImageRenderer::New();
    imageRenderer->addInputConnection(importer->getOutputPort());

    SegmentationRenderer::pointer renderer = SegmentationRenderer::New();
    renderer->addInputConnection(segmentation->getOutputPort());

    SimpleWindow::pointer window = SimpleWindow::New();
    window->set2DMode();
    window->addRenderer(imageRenderer);
    window->addRenderer(renderer);
    window->setTimeout(2000);
    CHECK_NOTHROW(window->start());
}


TEST_CASE("SegmentationRenderer on a stream of thresholded 2D images with draw contours only", "[fast][SegmentationRenderer][visual]") {
    ImageFileStreamer::pointer importer = ImageFileStreamer::New();
    importer->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-2Dt/US-2Dt_#.mhd");

    BinaryThresholding::pointer segmentation = BinaryThresholding::New();
    segmentation->setInputConnection(importer->getOutputPort());
    segmentation->setLowerThreshold(100);

    ImageRenderer::pointer imageRenderer = ImageRenderer::New();
    imageRenderer->addInputConnection(importer->getOutputPort());

    SegmentationRenderer::pointer renderer = SegmentationRenderer::New();
    renderer->addInputConnection(segmentation->getOutputPort());
    renderer->setFillArea(false);

    SimpleWindow::pointer window = SimpleWindow::New();
    window->set2DMode();
    window->addRenderer(imageRenderer);
    window->addRenderer(renderer);
    window->setTimeout(2000);
    CHECK_NOTHROW(window->start());
}

TEST_CASE("SegmentationRenderer on a stream of thresholded 2D images and set different color", "[fast][SegmentationRenderer][visual]") {
    ImageFileStreamer::pointer importer = ImageFileStreamer::New();
    importer->setFilenameFormat(std::string(FAST_TEST_DATA_DIR) + "US-2Dt/US-2Dt_#.mhd");

    BinaryThresholding::pointer segmentation = BinaryThresholding::New();
    segmentation->setInputConnection(importer->getOutputPort());
    segmentation->setLowerThreshold(100);

    ImageRenderer::pointer imageRenderer = ImageRenderer::New();
    imageRenderer->addInputConnection(importer->getOutputPort());

    SegmentationRenderer::pointer renderer = SegmentationRenderer::New();
    renderer->addInputConnection(segmentation->getOutputPort());
    renderer->setColor(Segmentation::LABEL_FOREGROUND, Color::Red());

    SimpleWindow::pointer window = SimpleWindow::New();
    window->set2DMode();
    window->addRenderer(imageRenderer);
    window->addRenderer(renderer);
    window->setTimeout(2000);
    CHECK_NOTHROW(window->start());
}


