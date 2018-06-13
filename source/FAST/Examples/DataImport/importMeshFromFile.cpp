/**
 * Examples/DataImport/importMeshFromFile.cpp
 *
 * If you edit this example, please also update the wiki and source code file in the repository.
 */
#include <FAST/Tools/CommandLineParser.hpp>
#include "FAST/Importers/VTKMeshFileImporter.hpp"
#include "FAST/Visualization/TriangleRenderer/TriangleRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"

using namespace fast;

int main(int argc, char** argv) {
    CommandLineParser parser("Import triangle mesh from file");
    parser.addPositionVariable(1, "filename", Config::getTestDataPath()+"/Surface_LV.vtk");
    parser.parse(argc, argv);

    // Import a triangle mesh from vtk file using the VTKMeshFileImporter
    VTKMeshFileImporter::pointer importer = VTKMeshFileImporter::New();
    importer->setFilename(parser.get("filename"));

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
