/**
 * Examples/DataImport/importPointSetFromFile.cpp
 *
 * If you edit this example, please also update the wiki and source code file in the repository.
 */
#include <FAST/Tools/CommandLineParser.hpp>
#include "FAST/Importers/VTKMeshFileImporter.hpp"
#include "FAST/Visualization/VertexRenderer/VertexRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"

using namespace fast;

int main(int argc, char** argv) {
    CommandLineParser parser("Import point set from file");
    parser.addPositionVariable(1, "filename", Config::getTestDataPath() + "Surface_LV.vtk");
    parser.parse(argc, argv);

    // Import line set from vtk file
    VTKMeshFileImporter::pointer importer = VTKMeshFileImporter::New();
    importer->setFilename(parser.get("filename"));

    // Render vertices
    VertexRenderer::pointer renderer = VertexRenderer::New();
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
