/**
 * Examples/DataImport/streamImagesFromDisk.cpp
 *
 * If you edit this example, please also update the wiki and source code file in the repository.
 */
#include <FAST/Tools/CommandLineParser.hpp>
#include "FAST/Streamers/ImageFileStreamer.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"

using namespace fast;

int main(int argc, char** argv) {
    CommandLineParser parser("Stream images from disk");
    // The hashtag here will be replaced with an integer, starting with 0 as default
    parser.addPositionVariable(1, "filename", Config::getTestDataPath() + "/US/CarotidArtery/Right/US-2D_#.mhd");
    parser.addVariable("sleep-time", "50");
    parser.addOption("render-in-3d");
    parser.parse(argc, argv);

    auto streamer = ImageFileStreamer::New();
    streamer->setFilenameFormat(parser.get("filename"));
    streamer->setSleepTime(parser.get<int>("sleep-time"));

    auto renderer = ImageRenderer::New();
    renderer->addInputConnection(streamer->getOutputPort());

    auto window = SimpleWindow::New();
    window->addRenderer(renderer);
    if(!parser.getOption("render-in-3d"))
        window->set2DMode();
#ifdef FAST_CONTINUOUS_INTEGRATION
    // This will automatically close the window after 6 seconds, used for CI testing
    window->setTimeout(5*1000);
#endif
    window->start();
}
