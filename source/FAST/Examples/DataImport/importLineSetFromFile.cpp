/**
 * Examples/DataImport/importLineSetFromFile.cpp
 *
 * If you edit this example, please also update the wiki and source code file in the repository.
 */
#include "FAST/Importers/VTKMeshFileImporter.hpp"
#include "FAST/Visualization/LineRenderer/LineRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"


using namespace fast;

int main() {
    // Import line set from vtk file
    VTKMeshFileImporter::pointer importer = VTKMeshFileImporter::New();
    importer->setFilename(Config::getTestDataPath() + "centerline.vtk");

    // Renderer image
    LineRenderer::pointer renderer = LineRenderer::New();
    renderer->addInputConnection(importer->getOutputPort());
    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
#ifdef FAST_CONTINUOUS_INTEGRATION
	// This will automatically close the window after 5 seconds, used for CI testing
    window->setTimeout(5*1000);
#endif
    window->start();
}
