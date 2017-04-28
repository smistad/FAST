/**
 * Examples/DataImport/importPointSetFromFile.cpp
 *
 * If you edit this example, please also update the wiki and source code file in the repository.
 */
#include "FAST/Importers/VTKPointSetFileImporter.hpp"
#include "FAST/Visualization/PointRenderer/PointRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"


using namespace fast;

int main() {
    // Import line set from vtk file
    VTKPointSetFileImporter::pointer importer = VTKPointSetFileImporter::New();
    importer->setFilename(Config::getTestDataPath() + "Surface_LV.vtk");

    // Renderer image
    PointRenderer::pointer renderer = PointRenderer::New();
    renderer->addInputConnection(importer->getOutputPort());
    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
#ifdef FAST_CONTINUOUS_INTEGRATION
	// This will automatically close the window after 5 seconds, used for CI testing
    window->setTimeout(5*1000);
#endif
    window->start();
}
