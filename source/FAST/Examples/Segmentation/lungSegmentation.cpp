/**
 * Examples/Segmentation/binaryThresholding.cpp
 *
 * If you edit this example, please also update the wiki and source code file in the repository.
 */
#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/Algorithms/SeededRegionGrowing/SeededRegionGrowing.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SegmentationRenderer/SegmentationRenderer.hpp"
#include "FAST/Visualization/MeshRenderer/MeshRenderer.hpp"
#include "FAST/Algorithms/SurfaceExtraction/SurfaceExtraction.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"
#include "FAST/Algorithms/Morphology/Dilation.hpp"
#include "FAST/Algorithms/Morphology/Erosion.hpp"


using namespace fast;

int main() {
    // Import image from file using the ImageFileImporter
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(Config::getTestDataPath()+"CT/CT-Thorax.mhd");

    // Segment image
    SeededRegionGrowing::pointer segmentation = SeededRegionGrowing::New();
    segmentation->setInputConnection(importer->getOutputPort());
    segmentation->addSeedPoint(166, 220, 415);
    segmentation->setIntensityRange(-900, -500);

    Dilation::pointer dilation = Dilation::New();
    dilation->setInputConnection(segmentation->getOutputPort());
    dilation->setStructuringElementSize(15);

    Erosion::pointer erosion = Erosion::New();
    erosion->setInputConnection(dilation->getOutputPort());

    // Renderer segmentation on top of input image
    ImageRenderer::pointer imageRenderer = ImageRenderer::New();
    imageRenderer->addInputConnection(importer->getOutputPort());
    SegmentationRenderer::pointer segmentationRenderer = SegmentationRenderer::New();
    segmentationRenderer->addInputConnection(erosion->getOutputPort());

    SurfaceExtraction::pointer extraction = SurfaceExtraction::New();
    extraction->setInputConnection(erosion->getOutputPort());

    MeshRenderer::pointer meshRenderer = MeshRenderer::New();
    meshRenderer->addInputConnection(extraction->getOutputPort());

    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(imageRenderer);
    window->addRenderer(segmentationRenderer);
    window->set2DMode();
    //window->addRenderer(meshRenderer);
#ifdef FAST_CONTINUOUS_INTEGRATION
    // This will automatically close the window after 5 seconds, used for CI testing
    window->setTimeout(5*1000);
#endif
    window->start();
}
