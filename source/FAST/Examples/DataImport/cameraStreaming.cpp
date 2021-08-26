/**
 * @example cameraStreaming.cpp
 * Stream from a (web) camera connected to your computer
 */
#include <FAST/Streamers/CameraStreamer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Tools/CommandLineParser.hpp>

using namespace fast;

int main(int argc, char ** argv) {
    CommandLineParser parser("Camera streaming example");
    parser.addVariable("index", "0", "Camera index");
    parser.addOption("grayscale", "Convert incoming camera images to grayscale");
    parser.parse(argc, argv);

    auto streamer = CameraStreamer::create(parser.getOption("grayscale"), parser.get<int>("index"));

    auto renderer = ImageRenderer::create()
            ->connect(streamer);

    auto window = SimpleWindow2D::create()
            ->connect(renderer);
    window->run();
}