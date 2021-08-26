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
    parser.parse(argc, argv);

    auto streamer = ClariusStreamer::create();

    auto renderer = ImageRenderer::create()
            ->connect(streamer);

    auto window = SimpleWindow2D::create()
            ->connect(renderer);
    window->getView()->setAutoUpdateCamera(true);
    window->run();
}