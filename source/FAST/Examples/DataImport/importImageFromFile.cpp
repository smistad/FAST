/**
 * Examples/DataImport/importImageFromFile.cpp
 *
 * If you edit this example, please also update the wiki and source code file in the repository.
 */
#include <FAST/Tools/CommandLineParser.hpp>
#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"

using namespace fast;

int main(int argc, char** argv) {
    Reporter::setGlobalReportMethod(Reporter::COUT);
    CommandLineParser parser("Import image from file example");
    parser.addPositionVariable(1, "filename", Config::getTestDataPath() + "US/US-2D.jpg");
    parser.parse(argc, argv);

    // Import image from file using the ImageFileImporter
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(parser.get(1));

    // Render
    ImageRenderer::pointer renderer = ImageRenderer::New();
    renderer->addInputConnection(importer->getOutputPort());

    // Setup window
    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->set2DMode();
#ifdef FAST_CONTINUOUS_INTEGRATION
	// This will automatically close the window after 5 seconds, used for CI testing
    window->setTimeout(5*1000);
#endif
    window->start();
}
