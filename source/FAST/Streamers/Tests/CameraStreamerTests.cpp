#include <FAST/Testing.hpp>
#include <FAST/Streamers/CameraStreamer.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include <QMediaDevices>
#include <FAST/Data/Image.hpp>

using namespace fast;

TEST_CASE("CameraStreamer window", "[fast][camerastreamer][visual]") {
    Window::initializeQtApp(); // Must initialize qt before video inputs
	auto cameras = QMediaDevices::videoInputs();
    if(!cameras.empty()) {
        auto streamer = CameraStreamer::create();

        auto renderer = ImageRenderer::create()->connect(streamer);

        auto window = SimpleWindow2D::create()->connect(renderer);
        window->setTimeout(2000);
        window->start();
    }
}

TEST_CASE("CameraStreamer stream", "[fast][camerastreamer][visual]") {
    Window::initializeQtApp(); // Must initialize qt before video inputs
    auto cameras = QMediaDevices::videoInputs();
    if(!cameras.empty()) {
        auto streamer = CameraStreamer::create();

        auto stream = DataStream(streamer);

        auto image = stream.getNextFrame<Image>();
        std::cout << "Got image with size: " << image->getWidth() << " " << image->getHeight() << std::endl;
    }
}