/**
 * Examples/Segmentation/binaryThresholding.cpp
 *
 * If you edit this example, please also update the wiki and source code file in the repository.
 */
#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/Algorithms/BinaryThresholding/BinaryThresholding.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SegmentationRenderer/SegmentationRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"

using namespace fast;

int main() {
    // Import image from file using the ImageFileImporter
    auto importer = ImageFileImporter::New();
    importer->setFilename(Config::getTestDataPath()+"US/CarotidArtery/Right/US-2D_100.mhd");

    // Segment image
    auto thresholding = BinaryThresholding::New();
    thresholding->setInputConnection(importer->getOutputPort());
    thresholding->setLowerThreshold(60);

    // Renderer segmentation on top of input image
    auto imageRenderer = ImageRenderer::New();
    imageRenderer->addInputConnection(importer->getOutputPort());

    auto segmentationRenderer = SegmentationRenderer::New();
    segmentationRenderer->addInputConnection(thresholding->getOutputPort());

    auto window = SimpleWindow::New();
    window->addRenderer(imageRenderer);
    window->addRenderer(segmentationRenderer);
    window->set2DMode();
#ifdef FAST_CONTINUOUS_INTEGRATION
	// This will automatically close the window after 5 seconds, used for CI testing
    window->setTimeout(5*1000);
#endif
    window->start();
}
