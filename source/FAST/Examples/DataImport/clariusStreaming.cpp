#include <FAST/Streamers/ClariusStreamer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>

using namespace fast;

int main() {
    Reporter::setGlobalReportMethod(Reporter::COUT);
    auto streamer = ClariusStreamer::New();

    auto renderer = ImageRenderer::New();
    renderer->addInputConnection(streamer->getOutputPort());

    auto window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->getView()->setAutoUpdateCamera(true);
    window->set2DMode();
    window->start(STREAMING_MODE_NEWEST_FRAME_ONLY);
}