#include "FAST/Testing.hpp"
#include "UltrasoundImageEnhancement.hpp"
#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"

namespace fast {

TEST_CASE("Ultrasound image enhancement", "[fast][ultrasoundimageenhancement]") {
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(Config::getTestDataPath() + "US/Heart/ApicalFourChamber/US-2D_0.mhd");

    UltrasoundImageEnhancement::pointer enhancement = UltrasoundImageEnhancement::New();
    enhancement->setInputConnection(importer->getOutputPort());

    ImageRenderer::pointer renderer = ImageRenderer::New();
    renderer->addInputConnection(enhancement->getOutputPort());

    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->set2DMode();
    window->setTimeout(500),
    window->start();

}

}