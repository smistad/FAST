/**
 * Examples/DataImport/kinectStreaming.cpp
 *
 * If you edit this example, please also update the wiki and source code file in the repository.
 */
#include "FAST/Streamers/KinectStreamer.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"


using namespace fast;

int main(int argc, char** argv) {
    KinectStreamer::pointer streamer = KinectStreamer::New();

    // Renderer image
    ImageRenderer::pointer renderer = ImageRenderer::New();
    renderer->addInputConnection(streamer->getOutputPort());
    renderer->setIntensityLevel(1000);
    renderer->setIntensityWindow(500);
    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->set2DMode();
#ifdef FAST_CONTINUOUS_INTEGRATION
    // This will automatically close the window after 5 seconds, used for CI testing
    window->setTimeout(5*1000);
#endif
    window->start();
}
