/**
 * @example importVertexMeshFromFile.cpp
 * An example of importing and visualizing a mesh containing vertices from a file using the VTKMeshFileImporter
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
    auto importer = VTKMeshFileImporter::create(parser.get("filename"));

    // Render vertices
    auto renderer = VertexRenderer::create()->connect(importer);

    // Setup window
    auto window = SimpleWindow3D::create()->connect(renderer);
#ifdef FAST_CONTINUOUS_INTEGRATION
	// This will automatically close the window after 5 seconds, used for CI testing
    window->setTimeout(5*1000);
#endif
    // Run entire pipeline and display window
    window->run();
}
