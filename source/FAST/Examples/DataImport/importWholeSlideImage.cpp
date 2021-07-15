/**
 * @example importWholeSlideImage.cpp
 * An example of importing and visualizing an image from file using the WholeSlideImageImporter
 */
#include <FAST/Tools/CommandLineParser.hpp>
#include <FAST/Importers/WholeSlideImageImporter.hpp>
#include <FAST/Visualization/ImagePyramidRenderer/ImagePyramidRenderer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>

using namespace fast;

int main(int argc, char** argv) {
    //Reporter::setGlobalReportMethod(Reporter::COUT);
    CommandLineParser parser("Import image from file example");
    parser.addPositionVariable(1, "filename", Config::getTestDataPath() + "/WSI/A05.svs");
    parser.parse(argc, argv);

    // Import image from file
    auto importer = WholeSlideImageImporter::create(parser.get(1));

    // Render
    auto renderer = ImagePyramidRenderer::create()->connect(importer);

    // Setup window
    auto window = SimpleWindow2D::create()->connect(renderer);
#ifdef FAST_CONTINUOUS_INTEGRATION
    // This will automatically close the window after 5 seconds, used for CI testing
    window->setTimeout(5*1000);
#endif
    // Run entire pipeline and display window
    window->run();
}
