#include "FAST/Tests/catch.hpp"
#include "NonLocalMeans.hpp"
#include <FAST/Streamers/ImageFileStreamer.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Visualization/DualViewWindow.hpp>
#include <FAST/Algorithms/UltrasoundImageEnhancement/UltrasoundImageEnhancement.hpp>

using namespace fast;

TEST_CASE("Non local means", "[fast][nlm][visual]") {
    auto streamer = ImageFileStreamer::New();
    streamer->setFilenameFormat(Config::getTestDataPath() + "US/Heart/ApicalFourChamber/US-2D_#.mhd");
    streamer->enableLooping();

    auto filter = NonLocalMeans::New();
    filter->setInputConnection(streamer->getOutputPort());

    auto enhance = UltrasoundImageEnhancement::New();
    enhance->setInputConnection(filter->getOutputPort());

    auto renderer = ImageRenderer::create()->connect(enhance);

    auto enhance2 = UltrasoundImageEnhancement::create()->connect(streamer);

    auto renderer2 = ImageRenderer::create()->connect(enhance2);

    auto window = DualViewWindow2D::create(Color::Black())
            ->connectLeft(renderer)
            ->connectRight(renderer2);
    window->setTimeout(2000);
    window->run();
}
