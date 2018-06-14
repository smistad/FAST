/**
 * Examples/DataImport/importLineSetFromFile.cpp
 *
 * If you edit this example, please also update the wiki and source code file in the repository.
 */
#include <FAST/Tools/CommandLineParser.hpp>
#include "FAST/Importers/VTKMeshFileImporter.hpp"
#include "FAST/Visualization/LineRenderer/LineRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"

using namespace fast;

int main(int argc, char** argv) {
    CommandLineParser parser("Import line set from file");
    parser.addPositionVariable(1, "filename", Config::getTestDataPath() + "centerline.vtk");
    parser.parse(argc, argv);

    // Import line set from vtk file
    VTKMeshFileImporter::pointer importer = VTKMeshFileImporter::New();
    importer->setFilename(parser.get("filename"));

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
