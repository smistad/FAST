/**
 * Examples/DataImport/importMeshFromFile.cpp
 *
 * If you edit this example, please also update the wiki and source code file in the repository.
 */
#include "FAST/Importers/VTKMeshFileImporter.hpp"
#include "FAST/Visualization/MeshRenderer/MeshRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"

using namespace fast;

int main() {
    // Import mesh from vtk file using the VTKMeshFileImporter
    VTKMeshFileImporter::pointer importer = VTKMeshFileImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR)+"/Surface_LV.vtk");

    // Renderer image
    MeshRenderer::pointer renderer = MeshRenderer::New();
    renderer->addInputConnection(importer->getOutputPort());
    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->setTimeout(5*1000); // automatically close window after 5 seconds
    window->start();
}
