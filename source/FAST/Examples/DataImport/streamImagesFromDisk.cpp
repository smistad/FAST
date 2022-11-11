/**
 * @example streamImagesFromDisk.cpp
 * An example of streaming and visualizing images from files using the ImageFileStreamer
 */
#include <FAST/Tools/CommandLineParser.hpp>
#include <FAST/Visualization/Widgets/PlaybackWidget.hpp>
#include "FAST/Streamers/ImageFileStreamer.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"

using namespace fast;

int main(int argc, char** argv) {
    CommandLineParser parser("Stream images from disk");
    // The hashtag here will be replaced with an integer, starting with 0 as default
    parser.addPositionVariable(1, "filename", Config::getTestDataPath() + "/US/CarotidArtery/Right/US-2D_#.mhd");
    parser.addVariable("framerate", "30", "Framerate");
    parser.addOption("render-in-3d");
    parser.parse(argc, argv);

    auto streamer = ImageFileStreamer::create(parser.get("filename"));
    if(parser.gotValue("framerate"))
        streamer->setFramerate(parser.get<int>("framerate"));

    auto widget = new PlaybackWidget(streamer);

    auto renderer = ImageRenderer::create()->connect(streamer);

    auto window = SimpleWindow::create()
            ->connect(renderer)
            ->connect(widget);
    if(!parser.getOption("render-in-3d"))
        window->set2DMode();
#ifdef FAST_CONTINUOUS_INTEGRATION
    // This will automatically close the window after 6 seconds, used for CI testing
    window->setTimeout(5*1000);
#endif
    // Run entire pipeline and display window
    window->run();
}
