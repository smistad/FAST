/**
 * Examples/Filtering/GaussianSmoothing.cpp
 *
 * If you edit this example, please also update the wiki and source code file in the repository.
 */
#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/Algorithms/GaussianSmoothing/GaussianSmoothing.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"


using namespace fast;

int main() {
    // Import image from file using the ImageFileImporter
    auto importer = ImageFileImporter::create(Config::getTestDataPath()+"/US/US-2D.jpg");

    // Smooth image
    auto filter = GaussianSmoothing::create(2.0, 7)->connect(importer);

    // Renderer image
    auto renderer = ImageRenderer::create()->connect(filter);

    auto window = SimpleWindow2D::create()->connect(renderer);
    window->addRenderer(renderer);
#ifdef FAST_CONTINUOUS_INTEGRATION
	// This will automatically close the window after 5 seconds, used for CI testing
    window->setTimeout(5*1000);
#endif
    window->run();
}
