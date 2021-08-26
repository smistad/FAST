/**
 * @example realSenseStreaming.cpp
 */
#include "FAST/Streamers/RealSenseStreamer.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/VertexRenderer/VertexRenderer.hpp"
#include "FAST/Visualization/MultiViewWindow.hpp"

using namespace fast;

int main(int argc, char** argv) {
    // Setup streaming
    auto streamer = RealSenseStreamer::create();

    // Renderer RGB image
    auto renderer = ImageRenderer::create()->connect(streamer);

    // Renderer depth image
    auto renderer2 = ImageRenderer::create(1000, 500)->connect(streamer, 1);

    // Render point cloud
    auto renderer3 = VertexRenderer::create(1.5)->connect(streamer, 2);

    // Setup window
    auto window = MultiViewWindow::create(3, Color::Black(), 1920, 512)
            ->connect(0, renderer)
            ->connect(1, renderer2)
            ->connect(2, renderer3);
    window->setTitle("FAST Real Sense Streaming");
    window->getView(0)->set2DMode();
    window->getView(1)->set2DMode();
    window->getView(2)->set3DMode();
    // Adjust camera
    window->getView(2)->setLookAt(Vector3f(0,-500,-500), Vector3f(0,0,1000), Vector3f(0,-1,0), 1, 5000);
    window->getView(2)->setBackgroundColor(Color::Black());
    window->enableFullscreen();
#ifdef FAST_CONTINUOUS_INTEGRATION
    // This will automatically close the window after 5 seconds, used for CI testing
    window->setTimeout(5*1000);
#endif
    window->run();
}
