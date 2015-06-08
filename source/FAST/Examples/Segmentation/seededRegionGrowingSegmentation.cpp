/**
 * Examples/Segmentation/seededRegionGrowingSegmentation.cpp
 *
 * If you edit this example, please also update the wiki and source code file in the repository.
 */
#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/Algorithms/SeededRegionGrowing/SeededRegionGrowing.hpp"
#include "FAST/Algorithms/SurfaceExtraction/SurfaceExtraction.hpp"
#include "FAST/Visualization/MeshRenderer/MeshRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"

using namespace fast;

int main() {
    // Import CT image
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR) + "CT-Abdomen.mhd");

    // Perform region growing segmentation
    SeededRegionGrowing::pointer segmentation = SeededRegionGrowing::New();
    segmentation->setInputConnection(importer->getOutputPort());
    segmentation->addSeedPoint(223,282,387);
    segmentation->addSeedPoint(251,314,148);
    segmentation->setIntensityRange(150, 5000);

    // Extraction surface mesh from segmentation
    SurfaceExtraction::pointer extraction = SurfaceExtraction::New();
    extraction->setInputConnection(segmentation->getOutputPort());

    // Render and visualize the mesh
    MeshRenderer::pointer surfaceRenderer = MeshRenderer::New();
    surfaceRenderer->setInputConnection(extraction->getOutputPort());
    surfaceRenderer->enableRuntimeMeasurements();

	SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(surfaceRenderer);
    window->setTimeout(5*1000); // automatically close window after 5 seconds
    window->start();
}
