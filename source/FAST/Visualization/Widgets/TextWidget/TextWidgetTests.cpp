#include <FAST/Testing.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include "TextWidget.hpp"

using namespace fast;

TEST_CASE("Text widget", "[fast][TextWidget][visual]") {
    auto renderer = ImageRenderer::create();

    auto widget = new TextWidget("<h1>Test</h1>{value}");
    widget->setVariable("value", "Cool!");

    auto window = SimpleWindow2D::create()
        ->connect(renderer)
        ->connect(widget);
    window->setTimeout(2000);
    window->run();
}