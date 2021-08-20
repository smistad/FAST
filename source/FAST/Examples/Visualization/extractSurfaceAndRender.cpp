/**
 * @example extractSurfaceAndRender.cpp
 */
#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/Algorithms/SurfaceExtraction/SurfaceExtraction.hpp"
#include "FAST/Visualization/TriangleRenderer/TriangleRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"


using namespace fast;

int main() {
    // Import CT image
    auto importer = ImageFileImporter::create(Config::getTestDataPath() + "CT/CT-Abdomen.mhd");

    // Extract surface mesh using a threshold value
    auto extraction = SurfaceExtraction::create(300)->connect(importer);

    // Render and visualize the mesh
    auto surfaceRenderer = TriangleRenderer::create()->connect(extraction);

	auto window = SimpleWindow3D::create()->connect(surfaceRenderer);
#ifdef FAST_CONTINUOUS_INTEGRATION
	// This will automatically close the window after 5 seconds, used for CI testing
    window->setTimeout(5*1000);
#endif
    window->run();
}
