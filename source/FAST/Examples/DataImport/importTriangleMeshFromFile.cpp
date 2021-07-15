/**
 * @example importTriangleMeshFromFile.cpp
 * An example of importing and visualizing a mesh containing triangles from a file using the VTKMeshFileImporter
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
    auto importer = VTKMeshFileImporter::create(parser.get("filename"));

    // Renderer mesh
    auto renderer = TriangleRenderer::create()->connect(importer);

    // Setup window
    auto window = SimpleWindow3D::create()->connect(renderer);
#ifdef FAST_CONTINUOUS_INTEGRATION
    window->setTimeout(5*1000); // automatically close window after 5 seconds
#endif
    // Run entire pipeline and display window
    window->run();
}
