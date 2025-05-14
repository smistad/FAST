#include <FAST/Testing.hpp>
#include <FAST/Streamers/VideoRandomAccessStreamer.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Visualization/Widgets/PlaybackWidget/PlaybackWidget.hpp>

using namespace fast;

TEST_CASE("VideoRandomAccessStreamer", "[fast][VideoRandomAccessStreamer][visual]") {
    auto streamer = VideoRandomAccessStreamer::create(
            Config::getTestDataPath() + "US/sagittal_spine.avi",
            true,
            true
    );

    auto renderer = ImageRenderer::create()->connect(streamer);

    auto playbackWidget = new PlaybackWidget(streamer);
    auto window = SimpleWindow2D::create()->connect(renderer);
    window->connect(playbackWidget);
    window->setTimeout(2000);
    window->start();

}