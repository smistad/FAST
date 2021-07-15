/**
 * @example binaryThresholding.cpp
 *
 */
#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/Algorithms/BinaryThresholding/BinaryThresholding.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SegmentationRenderer/SegmentationRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"

using namespace fast;

int main() {
    // Import image from file using the ImageFileImporter
    auto importer = ImageFileImporter::create(Config::getTestDataPath()+"US/CarotidArtery/Right/US-2D_100.mhd");

    // Segment image
    auto thresholding = BinaryThresholding::create(60)->connect(importer);

    // Renderer segmentation on top of input image
    auto imageRenderer = ImageRenderer::create()->connect(importer);

    auto segmentationRenderer = SegmentationRenderer::create()->connect(thresholding);

    auto window = SimpleWindow2D::create()->connect({imageRenderer, segmentationRenderer});
#ifdef FAST_CONTINUOUS_INTEGRATION
	// This will automatically close the window after 5 seconds, used for CI testing
    window->setTimeout(5*1000);
#endif
    window->run();
}
