/**
 * Examples/Segmentation/lungSegmentation.cpp
 *
 * If you edit this example, please also update the wiki and source code file in the repository.
 */
#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SegmentationRenderer/SegmentationRenderer.hpp"
#include "FAST/Visualization/TriangleRenderer/TriangleRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"
#include "FAST/Algorithms/SurfaceExtraction/SurfaceExtraction.hpp"
#include "FAST/Algorithms/LungSegmentation/LungSegmentation.hpp"


using namespace fast;

int main(int argc, char** argv) {
    // Import image from file using the ImageFileImporter
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    if(argc > 1) {
        importer->setFilename(argv[1]);
    } else {
        importer->setFilename(Config::getTestDataPath() + "CT/CT-Thorax.mhd");
    }

    // Perform lung segmentation (this will also extract the airways using AirwaySegmentation)
    LungSegmentation::pointer segmentation = LungSegmentation::New();
    segmentation->setInputConnection(importer->getOutputPort());

    // Extract lung surface
    SurfaceExtraction::pointer extraction = SurfaceExtraction::New();
    extraction->setInputConnection(segmentation->getOutputPort());

    // Extract airway surface
    SurfaceExtraction::pointer extraction2 = SurfaceExtraction::New();
    extraction2->setInputConnection(segmentation->getOutputPort(1));

    // Render both surfaces with different color
    TriangleRenderer::pointer TriangleRenderer = TriangleRenderer::New();
    TriangleRenderer->addInputConnection(extraction->getOutputPort(), Color::Green(), 0.6f);
    TriangleRenderer->addInputConnection(extraction2->getOutputPort(), Color::Red(), 1.0f);

    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(TriangleRenderer);
#ifdef FAST_CONTINUOUS_INTEGRATION
    // This will automatically close the window after 5 seconds, used for CI testing
    window->setTimeout(5*1000);
#endif
    window->start();
}
