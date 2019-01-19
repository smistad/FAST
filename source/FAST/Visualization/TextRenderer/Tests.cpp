#include <FAST/Testing.hpp>
#include "TextRenderer.hpp"
#include <FAST/Visualization/SimpleWindow.hpp>

using namespace fast;

TEST_CASE("Text renderer", "[fast][TextRenderer][visual]") {

    auto text = Text::New();
    text->create("This is a test!");

    auto renderer = TextRenderer::New();
    renderer->addInputData(text);
    renderer->setFontSize(64);
    //renderer->setStyle(TextRenderer::STYLE_ITALIC);


    auto window = SimpleWindow::New();
    renderer->setView(window->getView());
    window->addRenderer(renderer);
    window->getView()->setBackgroundColor(Color::Black());
    window->set2DMode();
    window->setTimeout(1000);
    window->start();
}