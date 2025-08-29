/**
 * @example clariusStreaming.cpp
 */
#include <FAST/Streamers/ClariusStreamer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Tools/CommandLineParser.hpp>
#include <FAST/Visualization/Widgets/ButtonWidget/ButtonWidget.hpp>
#include <FAST/Visualization/Widgets/SliderWidget/SliderWidget.hpp>

using namespace fast;

int main(int argc, char**argv) {
    CommandLineParser parser("Clarius streaming example");
    parser.addVariable("port", "5828", "Port to use for clarius connection");
    parser.addVariable("ip", "192.168.1.1", "Address to use for clarius connection");
    parser.parse(argc, argv);

    auto streamer = ClariusStreamer::create(parser.get("ip"), parser.get<int>("port"));

    auto freezeButton = new ButtonWidget("Freeze", true, [streamer](bool checked) {
        streamer->toggleFreeze();
    });

    // How to set initial?? What is min and what is max?
    auto depthSlider = new SliderWidget("Depth", 15, 1, 30, 1, [streamer](float value) {
        streamer->setDepth(value);
    });

    auto renderer = ImageRenderer::create()
            ->connect(streamer);

    auto window = SimpleWindow2D::create()
            ->connect(renderer);
    window->connect(freezeButton, WidgetPosition::RIGHT);
    window->connect(depthSlider, WidgetPosition::RIGHT);
    window->setTitle("FAST Clarius Streaming");
    window->getView()->setAutoUpdateCamera(true);
    window->run();
}