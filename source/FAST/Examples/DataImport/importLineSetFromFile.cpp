/**
 * Examples/DataImport/importLineSetFromFile.cpp
 *
 * If you edit this example, please also update the wiki and source code file in the repository.
 */
#include "FAST/Importers/VTKMeshFileImporter.hpp"
#include "FAST/Visualization/LineRenderer/LineRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"

using namespace fast;

int main(int argc, char** argv) {
    std::string path = Config::getTestDataPath() + "centerline.vtk";

    // Allow user to send in a custom path
    if(argc > 1) {
        if(std::string(argv[1]) == "--help") {
            std::cout << "usage: " << argv[0] << " [/path/to/data.vtk]" << "\n";
            return 0;
        }
        path = argv[1];
    }

    // Import line set from vtk file
    VTKMeshFileImporter::pointer importer = VTKMeshFileImporter::New();
    importer->setFilename(path);

    // Renderer
    LineRenderer::pointer renderer = LineRenderer::New();
    renderer->addInputConnection(importer->getOutputPort());

    // Setup window
    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
#ifdef FAST_CONTINUOUS_INTEGRATION
	// This will automatically close the window after 5 seconds, used for CI testing
    window->setTimeout(5*1000);
#endif
    window->start();
}
