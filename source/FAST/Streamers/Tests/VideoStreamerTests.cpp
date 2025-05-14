#include <FAST/Testing.hpp>
#include <FAST/Streamers/VideoStreamer.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>

using namespace fast;

TEST_CASE("VideoStreamer", "[fast][VideoStreamer][visual]") {
    auto streamer = VideoStreamer::create(
            Config::getTestDataPath() + "US/sagittal_spine.avi",
            true,
            false
    );

    auto renderer = ImageRenderer::create()->connect(streamer);

    auto window = SimpleWindow2D::create()->connect(renderer);
    window->setTimeout(2000);
    window->start();

}