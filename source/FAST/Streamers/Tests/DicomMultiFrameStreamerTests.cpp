#include <FAST/Testing.hpp>
#include <FAST/Streamers/DicomMultiFrameStreamer.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Visualization/Widgets/PlaybackWidget/PlaybackWidget.hpp>

using namespace fast;

TEST_CASE("DicomMultiFrameStreamer", "[fast][DicomMultiFrameStreamer][visual]") {
    auto streamer = DicomMultiFrameStreamer::create(
            "file",
            false,
            true,
            -1,
            true,
            true
    );

    CHECK_THROWS(
    auto playbackWidget = new PlaybackWidget(streamer);
    auto renderer = ImageRenderer::create()->connect(streamer);

    auto window = SimpleWindow2D::create()->connect(renderer)->connect(playbackWidget);
    window->setTimeout(2000);
    window->run();
    );
}
