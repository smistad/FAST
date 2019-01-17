#include <FAST/Testing.hpp>
#include "VolumeRenderer.hpp"
#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Importers/ImageFileImporter.hpp>

using namespace fast;

TEST_CASE("Volume renderer", "[fast][volumerenderer][visual]") {

    auto importer = ImageFileImporter::New();
    importer->setFilename(Config::getTestDataPath() + "CT/CT-Thorax.mhd");

    auto renderer = VolumeRenderer::New();
    renderer->addInputConnection(importer->getOutputPort());

    auto window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->start();
}