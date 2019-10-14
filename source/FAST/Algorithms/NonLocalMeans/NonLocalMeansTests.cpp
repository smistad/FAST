#include "FAST/Tests/catch.hpp"
#include "NonLocalMeans.hpp"
#include <FAST/Streamers/ImageFileStreamer.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Visualization/DualViewWindow.hpp>
#include <FAST/Algorithms/UltrasoundImageEnhancement/UltrasoundImageEnhancement.hpp>

using namespace fast;

TEST_CASE("Non local means", "[fast][nlm]") {
    auto streamer = ImageFileStreamer::New();
    streamer->setFilenameFormat(Config::getTestDataPath() + "US/Heart/ApicalFourChamber/US-2D_#.mhd");
    streamer->enableLooping();

    auto filter = NonLocalMeans::New();
    filter->setInputConnection(streamer->getOutputPort());

    auto enhance = UltrasoundImageEnhancement::New();
    enhance->setInputConnection(filter->getOutputPort());

    auto renderer = ImageRenderer::New();
    renderer->addInputConnection(enhance->getOutputPort());

    auto enhance2 = UltrasoundImageEnhancement::New();
    enhance2->setInputConnection(streamer->getOutputPort());

    auto renderer2 = ImageRenderer::New();
    renderer2->addInputConnection(enhance2->getOutputPort());

    auto window = DualViewWindow::New();
    window->addRendererToBottomRightView(renderer2);
    window->addRendererToTopLeftView(renderer);
    window->getView(0)->set2DMode();
    window->getView(1)->set2DMode();
    window->setTimeout(2000);
    window->start();
}
