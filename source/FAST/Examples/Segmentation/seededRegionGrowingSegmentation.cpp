/**
 * @example seededRegionGrowingSegmentation.cpp
 */
#include <FAST/Tools/CommandLineParser.hpp>
#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/Algorithms/SeededRegionGrowing/SeededRegionGrowing.hpp"
#include "FAST/Algorithms/SurfaceExtraction/SurfaceExtraction.hpp"
#include "FAST/Visualization/TriangleRenderer/TriangleRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"

using namespace fast;

int main(int argc, char** argv) {
    CommandLineParser parser("Seeded region growing");
    parser.addPositionVariable(1, "filename", Config::getTestDataPath() + "CT/CT-Abdomen.mhd");
    parser.addVariable("min-intensity", "150");
    parser.addVariable("max-intensity", "5000");
    parser.parse(argc, argv);

    // Import CT image
    auto importer = ImageFileImporter::create(parser.get("filename"));

    // Perform region growing segmentation
    auto segmentation = SeededRegionGrowing::create(parser.get<int>("min-intensity"), parser.get<int>("max-intensity"),
                  {
                        {223, 282, 387},
                        {251, 314, 148}
                  })->connect(importer);

    // Extraction surface mesh from segmentation
    auto extraction = SurfaceExtraction::create()->connect(segmentation);

    // Render and visualize the mesh
    auto surfaceRenderer = TriangleRenderer::create()->connect(extraction);

	auto window = SimpleWindow3D::create()->connect(surfaceRenderer);
#ifdef FAST_CONTINUOUS_INTEGRATION
	// This will automatically close the window after 5 seconds, used for CI testing
    window->setTimeout(5*1000);
#endif
    window->run();
}
