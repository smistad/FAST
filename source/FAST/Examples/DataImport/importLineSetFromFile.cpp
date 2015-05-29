/**
 * Examples/DataImport/importLineSetFromFile.cpp
 *
 * If you edit this example, please also update the wiki and source code file in the repository.
 */
#include "FAST/Importers/VTKLineSetFileImporter.hpp"
#include "FAST/Visualization/LineRenderer/LineRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"

using namespace fast;

int main() {
    // Import line set from vtk file
    VTKLineSetFileImporter::pointer importer = VTKLineSetFileImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR) + "centerline.vtk");

    // Renderer image
    LineRenderer::pointer renderer = LineRenderer::New();
    renderer->addInputConnection(importer->getOutputPort());
    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->setTimeout(5*1000); // automatically close window after 5 seconds
    window->start();
}
