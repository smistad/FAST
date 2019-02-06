#include <FAST/Testing.hpp>
#include <FAST/Algorithms/BlockMatching/BlockMatching.hpp>
#include <FAST/Streamers/ImageFileStreamer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Visualization/VectorFieldRenderer/VectorFieldRenderer.hpp>

using namespace fast;

TEST_CASE("Block matching 2D", "[fast][BlockMatching][visual]") {
    auto streamer = ImageFileStreamer::New();
    streamer->setFilenameFormat(Config::getTestDataPath() + "/US/Heart/ApicalFourChamber/US-2D_#.mhd");
    streamer->enableLooping();

    auto blockMatching = BlockMatching::New();
    blockMatching->setInputConnection(streamer->getOutputPort());
    blockMatching->setIntensityThreshold(75);
    blockMatching->setSearchSize(5);
    blockMatching->setBlockSize(9);
    blockMatching->enableRuntimeMeasurements();

    auto renderer = ImageRenderer::New();
    renderer->addInputConnection(streamer->getOutputPort());

    auto vectorRenderer = VectorFieldRenderer::New();
    vectorRenderer->addInputConnection(blockMatching->getOutputPort());

    auto window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->addRenderer(vectorRenderer);
    window->getView()->setBackgroundColor(Color::Black());
    window->set2DMode();
    //window->setTimeout(3000);
    window->start();
    blockMatching->getRuntime()->print();
}
