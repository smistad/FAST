#include <FAST/Testing.hpp>
#include <FAST/Streamers/CameraStreamer.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include <QtMultimedia/QCamera>

using namespace fast;

TEST_CASE("CameraStreamer", "[fast][camerastreamer][visual]") {
	auto cameras = QCameraInfo::availableCameras();
    if(!cameras.empty()) {
        auto streamer = CameraStreamer::New();

        auto renderer = ImageRenderer::New();
        renderer->addInputConnection(streamer->getOutputPort());

        auto window = SimpleWindow::New();
        window->addRenderer(renderer);
        window->set2DMode();
        window->setTimeout(1000);
        window->start();
    }
}