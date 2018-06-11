/**
 * Examples/DataImport/importMeshFromFile.cpp
 *
 * If you edit this example, please also update the wiki and source code file in the repository.
 */
#include "FAST/Importers/VTKMeshFileImporter.hpp"
#include "FAST/Visualization/TriangleRenderer/TriangleRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"

using namespace fast;

int main(int argc, char** argv) {
    std::string path = Config::getTestDataPath()+"/Surface_LV.vtk";

    // Allow user to send in a custom path
    if(argc > 1) {
        if(std::string(argv[1]) == "--help") {
            std::cout << "usage: " << argv[0] << " [/path/to/data.vtk]" << "\n";
            return 0;
        }
        path = argv[1];
    }

    // Import a triangle mesh from vtk file using the VTKMeshFileImporter
    VTKMeshFileImporter::pointer importer = VTKMeshFileImporter::New();
    importer->setFilename(path);

    // Renderer mesh
    TriangleRenderer::pointer renderer = TriangleRenderer::New();
    renderer->addInputConnection(importer->getOutputPort());

    // Setup window
    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
#ifdef FAST_CONTINUOUS_INTEGRATION
    window->setTimeout(5*1000); // automatically close window after 5 seconds
#endif
    window->start();
}
