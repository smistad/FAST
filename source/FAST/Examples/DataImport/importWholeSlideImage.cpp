/**
 * Examples/DataImport/importWholeSlideImage.cpp
 *
 * If you edit this example, please also update the wiki and source code file in the repository.
 */
#include <FAST/Tools/CommandLineParser.hpp>
#include "FAST/Importers/WholeSlideImageImporter.hpp"
#include "FAST/Visualization/ImagePyramidRenderer/ImagePyramidRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"

using namespace fast;

int main(int argc, char** argv) {
    Reporter::setGlobalReportMethod(Reporter::COUT);
    CommandLineParser parser("Import image from file example");
    parser.addPositionVariable(1, "filename", Config::getTestDataPath() + "/WSI/A05.svs");
    parser.parse(argc, argv);

    // Import image from file using the ImageFileImporter
    auto importer = WholeSlideImageImporter::New();
    importer->setFilename(parser.get(1));

    // Render
    auto renderer = ImagePyramidRenderer::New();
    renderer->addInputConnection(importer->getOutputPort());

    // Setup window
    auto window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->set2DMode();
#ifdef FAST_CONTINUOUS_INTEGRATION
    // This will automatically close the window after 5 seconds, used for CI testing
    window->setTimeout(5*1000);
#endif
    window->start();
}
