/**
 * @example clariusStreaming.cpp
 */
#include <FAST/Streamers/ClariusStreamer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Tools/CommandLineParser.hpp>

using namespace fast;

int main(int argc, char**argv) {
    CommandLineParser parser("Clarius streaming example");
    parser.addVariable("port", "5858", "Port to use for clarius connection");
    parser.addVariable("ip", "192.168.1.1", "Address to use for clarius connection");
    parser.parse(argc, argv);

    auto streamer = ClariusStreamer::create(parser.get("ip"), parser.get<int>("port"));

    auto renderer = ImageRenderer::create()
            ->connect(streamer);

    auto window = SimpleWindow2D::create()
            ->connect(renderer);
    window->getView()->setAutoUpdateCamera(true);
    window->run();
}