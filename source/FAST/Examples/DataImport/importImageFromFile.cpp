/**
 * Examples/DataImport/importImageFromFile.cpp
 *
 * If you edit this example, please also update the wiki and source code file in the repository.
 */
#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"

using namespace fast;

int main(int argc, char** argv) {
    std::string path = Config::getTestDataPath() + "US/US-2D.jpg";

    // Allow user to send in a custom path
    if(argc > 1) {
        if(std::string(argv[1]) == "--help") {
            std::cout << "usage: " << argv[0] << " [/path/to/image.mhd]" << "\n";
            return 0;
        }
        path = argv[1];
    }

    // Import image from file using the ImageFileImporter
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(path);

    // Render
    ImageRenderer::pointer renderer = ImageRenderer::New();
    renderer->addInputConnection(importer->getOutputPort());

    // Setup window
    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->set2DMode();
#ifdef FAST_CONTINUOUS_INTEGRATION
	// This will automatically close the window after 5 seconds, used for CI testing
    window->setTimeout(5*1000);
#endif
    window->start();
}
