#include <FAST/Testing.hpp>
#include <FAST/Importers/ImageFileImporter.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include "ButtonWidget.hpp"

using namespace fast;

TEST_CASE("Button widget", "[fast][ButtonWidget][visual]") {
    auto importer = ImageFileImporter::create(Config::getTestDataPath() + "US/Heart/ApicalFourChamber/US-2D_0.mhd");

    auto renderer = ImageRenderer::create()->connect(importer);

    auto widget = new ButtonWidget("Show renderer", true, [=](bool checked) {
        renderer->setDisabled(!renderer->isDisabled());
    });

    auto window = SimpleWindow2D::create()
        ->connect(renderer)
        ->connect(widget);
    window->setTimeout(2000);
    window->run();
}