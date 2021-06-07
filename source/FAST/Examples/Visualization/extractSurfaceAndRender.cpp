/**
 * Examples/Visualization/extractSurfaceAndRender.cpp
 *
 * If you edit this example, please also update the wiki and source code file in the repository.
 */
#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/Algorithms/SurfaceExtraction/SurfaceExtraction.hpp"
#include "FAST/Visualization/TriangleRenderer/TriangleRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"


using namespace fast;

int main() {
    // Import CT image
    auto importer = ImageFileImporter::New();
    importer->setFilename(Config::getTestDataPath() + "CT/CT-Abdomen.mhd");

    // Extract surface mesh using a threshold value
    auto extraction = SurfaceExtraction::create();
    extraction->setInputConnection(importer->getOutputPort());
    extraction->setThreshold(300);

    // Render and visualize the mesh
    auto surfaceRenderer = TriangleRenderer::New();
    surfaceRenderer->setInputConnection(extraction->getOutputPort());

	auto window = SimpleWindow::New();
    window->addRenderer(surfaceRenderer);
#ifdef FAST_CONTINUOUS_INTEGRATION
	// This will automatically close the window after 5 seconds, used for CI testing
    window->setTimeout(5*1000);
#endif
    window->start();
}
