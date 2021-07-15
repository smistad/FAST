/**
 * @example realSenseStreaming.cpp
 */
#include "FAST/Streamers/RealSenseStreamer.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/VertexRenderer/VertexRenderer.hpp"
#include "FAST/Visualization/MultiViewWindow.hpp"

using namespace fast;

int main(int argc, char** argv) {
    Config::setStreamingMode(STREAMING_MODE_NEWEST_FRAME_ONLY);
    // Setup streaming
    auto streamer = RealSenseStreamer::create();

    // Renderer RGB image
    auto renderer = ImageRenderer::create()->connect(streamer);

    // Renderer depth image
    auto renderer2 = ImageRenderer::create(1000, 500)->connect(streamer, 1);

    // Render point cloud
    auto renderer3 = VertexRenderer::create(1.5)->connect(streamer, 2);

    // Setup window
    auto window = MultiViewWindow::New();
    window->setTitle("FAST Real Sense Streaming");
    window->setHeight(512);
    window->setWidth(1920);
    window->setNrOfViews(3);
    window->addRenderer(0, renderer);
    window->getView(0)->set2DMode();
    window->getView(0)->setBackgroundColor(Color::Black());
    window->addRenderer(1, renderer2);
    window->getView(1)->set2DMode();
    window->getView(1)->setBackgroundColor(Color::Black());
    window->addRenderer(2, renderer3);
    // Adjust camera
    window->getView(2)->setLookAt(Vector3f(0,-500,-500), Vector3f(0,0,1000), Vector3f(0,-1,0), 1, 5000);
    window->getView(2)->setBackgroundColor(Color::Black());
    window->enableFullscreen();
#ifdef FAST_CONTINUOUS_INTEGRATION
    // This will automatically close the window after 5 seconds, used for CI testing
    window->setTimeout(5*1000);
#endif
    window->start();
}
