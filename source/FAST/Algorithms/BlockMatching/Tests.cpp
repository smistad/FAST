#include <FAST/Testing.hpp>
#include <FAST/Algorithms/BlockMatching/BlockMatching.hpp>
#include <FAST/Streamers/ImageFileStreamer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>

using namespace fast;

TEST_CASE("Block matching 2D", "[fast][BlockMatching][visual]") {
    auto streamer = ImageFileStreamer::New();
    streamer->setFilenameFormat(Config::getTestDataPath() + "/US/Heart/ApicalFourChamber/US-2D_#.mhd");
    streamer->enableLooping();
    streamer->setSleepTime(50);

    auto blockMatching = BlockMatching::New();
    blockMatching->setInputConnection(streamer->getOutputPort());

    auto renderer = ImageRenderer::New();
    renderer->addInputConnection(blockMatching->getOutputPort());

    auto window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->getView()->setBackgroundColor(Color::Black());
    window->set2DMode();
    window->setTimeout(1000);
    window->start();
}