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
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR)+"/US-2Dt/US-2Dt_0.mhd");

    // Segment image
    BinaryThresholding::pointer thresholding = BinaryThresholding::New();
    thresholding->setInputConnection(importer->getOutputPort());
    thresholding->setLowerThreshold(30);

    // Renderer segmentation on top of input image
    ImageRenderer::pointer imageRenderer = ImageRenderer::New();
    imageRenderer->addInputConnection(importer->getOutputPort());
    SegmentationRenderer::pointer segmentationRenderer = SegmentationRenderer::New();
    segmentationRenderer->addInputConnection(thresholding->getOutputPort());
    segmentationRenderer->setFillArea(false);
    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(imageRenderer);
    window->addRenderer(segmentationRenderer);
    window->set2DMode();
    window->setTimeout(5*1000); // automatically close window after 5 seconds
    window->start();
}
