/**
 * Examples/DataImport/streamImagesFromDisk.cpp
 *
 * If you edit this example, please also update the wiki and source code file in the repository.
 */
#include "FAST/Streamers/ImageFileStreamer.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"

using namespace fast;

int main(int argc, char** argv) {    
    // The hashtag here will be replaced with an integer, starting with 0 as default
    std::string path = Config::getTestDataPath() + "/US/CarotidArtery/Right/US-2D_#.mhd";
    
    // Allow user to send in a custom path
    if(argc > 1) {
        if(std::string(argv[1]) == "--help") {
            std::cout << "usage: " << argv[0] << " [/path/to/data/frame_#.mhd]\n";
            std::cout << "The files should be stored with an integer in place of the #: frame_0.mhd, frame_1.mhd ..." << std::endl;
            return 0;
        }
        path = argv[1];
    }

    ImageFileStreamer::pointer streamer = ImageFileStreamer::New();
    streamer->setFilenameFormat(path);
    streamer->setSleepTime(50);

    ImageRenderer::pointer renderer = ImageRenderer::New();
    renderer->addInputConnection(streamer->getOutputPort());

    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->set2DMode();
#ifdef FAST_CONTINUOUS_INTEGRATION
    // This will automatically close the window after 6 seconds, used for CI testing
    window->setTimeout(5*1000);
#endif
    window->start();
}
