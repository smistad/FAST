/**
 * Examples/DataImport/kinectStreaming.cpp
 *
 * If you edit this example, please also update the wiki and source code file in the repository.
 */
#include "FAST/Streamers/KinectStreamer.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/VertexRenderer/VertexRenderer.hpp"
#include "FAST/Visualization/MultiViewWindow.hpp"

using namespace fast;

int main(int argc, char** argv) {
    Config::setStreamingMode(STREAMING_MODE_NEWEST_FRAME_ONLY);
    Reporter::setGlobalReportMethod(Reporter::COUT);
    // Setup streaming
    KinectStreamer::pointer streamer1 = KinectStreamer::New();
    streamer1->setPointCloudFiltering(true);

    KinectStreamer::pointer streamer2 = KinectStreamer::New();
    streamer2->setPointCloudFiltering(true);

    // Render point cloud
    VertexRenderer::pointer renderer1 = VertexRenderer::New();
    renderer1->addInputConnection(streamer1->getOutputPort(2));
    renderer1->setDefaultSize(1.5);

    VertexRenderer::pointer renderer2 = VertexRenderer::New();
    renderer2->addInputConnection(streamer2->getOutputPort(2));
    renderer2->setDefaultSize(1.5);

    // Setup window
    MultiViewWindow::pointer window = MultiViewWindow::New();
    window->setTitle("FAST Kinect Streaming");
    window->setHeight(512);
    window->setWidth(1920);
    window->setNrOfViews(2);
    window->addRenderer(0, renderer1);
    window->getView(0)->setLookAt(Vector3f(0,-500,-500), Vector3f(0,0,1000), Vector3f(0,-1,0), 1, 5000);
    window->getView(0)->setBackgroundColor(Color::Black());
    window->addRenderer(1, renderer2);
    window->getView(1)->setLookAt(Vector3f(0,-500,-500), Vector3f(0,0,1000), Vector3f(0,-1,0), 1, 5000);
    window->getView(1)->setBackgroundColor(Color::Black());
#ifdef FAST_CONTINUOUS_INTEGRATION
    // This will automatically close the window after 5 seconds, used for CI testing
    window->setTimeout(5*1000);
#endif
    window->start();
}
