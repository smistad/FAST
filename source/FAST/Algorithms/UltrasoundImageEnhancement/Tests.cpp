#include "FAST/Testing.hpp"
#include "UltrasoundImageEnhancement.hpp"
#include "FAST/Streamers/ImageFileStreamer.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"

namespace fast {

TEST_CASE("Ultrasound image enhancement", "[fast][ultrasoundimageenhancement][visual]") {
    auto streamer = ImageFileStreamer::create(Config::getTestDataPath() + "US/Heart/ApicalFourChamber/US-2D_#.mhd");

    auto enhancement = UltrasoundImageEnhancement::create()->connect(streamer);

    ImageRenderer::pointer renderer = ImageRenderer::New();
    renderer->addInputConnection(enhancement->getOutputPort());

    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->set2DMode();
    window->setTimeout(500),
    window->start();

}

}