#include <FAST/Testing.hpp>
#include <FAST/Importers/ImageFileImporter.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include "SliderWidget.hpp"

using namespace fast;

TEST_CASE("Slider widget", "[fast][SliderWidget][visual]") {
    auto importer = ImageFileImporter::create(Config::getTestDataPath() + "US/Heart/ApicalFourChamber/US-2D_0.mhd");

    auto renderer = ImageRenderer::create()->connect(importer);

    auto widget = new SliderWidget("Level", 128, 0, 255, 2, [=](float value) {
        renderer->setIntensityLevel(value);
    });
    auto widget2 = new SliderWidget("Window", 255, 0, 255, 1, [=](float value) {
        renderer->setIntensityWindow(value);
    });

    auto window = SimpleWindow2D::create()
        ->connect(renderer)
        ->connect(widget)
        ->connect(widget2);
    window->setTimeout(2000);
    window->run();
}