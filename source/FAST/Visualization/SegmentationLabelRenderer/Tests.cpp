#include "SegmentationLabelRenderer.hpp"
#include <FAST/Testing.hpp>
#include <FAST/Streamers/ImageFileStreamer.hpp>
#include <FAST/Algorithms/BinaryThresholding/BinaryThresholding.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>

using namespace fast;

TEST_CASE("Segmentation label renderer", "[visual][SegmentationLabelRenderer][fast]") {
    auto importer = ImageFileStreamer::New();
    importer->setFilenameFormat(Config::getTestDataPath() + "US/CarotidArtery/Right/US-2D_#.mhd");

    auto segmentation = BinaryThresholding::New();
    segmentation->setInputConnection(importer->getOutputPort());
    segmentation->setLowerThreshold(100);

    auto imageRenderer = ImageRenderer::New();
    imageRenderer->addInputConnection(importer->getOutputPort());

    auto renderer = SegmentationLabelRenderer::New();
    renderer->addInputConnection(segmentation->getOutputPort());
    renderer->setLabelName(1, "Test");

    auto window = SimpleWindow::New();
    window->set2DMode();
    window->addRenderer(imageRenderer);
    window->addRenderer(renderer);
    window->setTimeout(2000);
    CHECK_NOTHROW(window->start());
}