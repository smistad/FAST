/**
 * Examples/DataImport/kinectStreaming.cpp
 *
 * If you edit this example, please also update the wiki and source code file in the repository.
 */
#include "FAST/Streamers/KinectStreamer.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/DualViewWindow.hpp"


using namespace fast;

int main(int argc, char** argv) {
    KinectStreamer::pointer streamer = KinectStreamer::New();

    // Renderer image
    ImageRenderer::pointer renderer = ImageRenderer::New();
    renderer->addInputConnection(streamer->getOutputPort());

    ImageRenderer::pointer renderer2 = ImageRenderer::New();
    renderer2->addInputConnection(streamer->getOutputPort(1));
    renderer2->setIntensityLevel(1000);
    renderer2->setIntensityWindow(500);

    DualViewWindow::pointer window = DualViewWindow::New();
    window->getTopLeftView()->addRenderer(renderer);
    window->getTopLeftView()->set2DMode();
    window->getBottomRightView()->addRenderer(renderer2);
    window->getBottomRightView()->set2DMode();
    window->enableFullscreen();
#ifdef FAST_CONTINUOUS_INTEGRATION
    // This will automatically close the window after 5 seconds, used for CI testing
    window->setTimeout(5*1000);
#endif
    window->start();
}
