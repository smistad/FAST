/**
 * Examples/Filtering/gaussianSmoothingFilter.cpp
 *
 * If you edit this example, please also update the wiki and source code file in the repository.
 */
#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/Algorithms/GaussianSmoothingFilter/GaussianSmoothingFilter.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"


using namespace fast;

int main() {
    // Import image from file using the ImageFileImporter
    auto importer = ImageFileImporter::New();
    importer->setFilename(Config::getTestDataPath()+"/US/US-2D.jpg");

    // Smooth image
    auto filter = GaussianSmoothingFilter::New();
    filter->setInputConnection(importer->getOutputPort());
    filter->setStandardDeviation(2.0);
    filter->setMaskSize(7);

    // Renderer image
    auto renderer = ImageRenderer::New();
    renderer->addInputConnection(filter->getOutputPort());

    auto window = SimpleWindow::New();
    window->addRenderer(renderer);
#ifdef FAST_CONTINUOUS_INTEGRATION
	// This will automatically close the window after 5 seconds, used for CI testing
    window->setTimeout(5*1000);
#endif
    window->start();
}
