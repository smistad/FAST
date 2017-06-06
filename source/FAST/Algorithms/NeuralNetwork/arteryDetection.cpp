#include "FAST/Streamers/IGTLinkStreamer.hpp"
#include "FAST/Algorithms/UltrasoundImageCropper/UltrasoundImageCropper.hpp"
#include "ObjectDetection.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/TriangleRenderer/TriangleRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"

using namespace fast;

int main() {
    // Connect to IGTLink server
    IGTLinkStreamer::pointer streamer = IGTLinkStreamer::New();
    streamer->setConnectionAddress("");
    streamer->setConnectionPort(18944);

    // Crop image
    UltrasoundImageCropper::pointer cropper = UltrasoundImageCropper::New();
    cropper->setInputConnection(streamer->getOutputPort<Image>("Image_Transducer"));

    // Send to object detection
    ObjectDetection::pointer detection = ObjectDetection::New();
    detection->setInputConnection(cropper->getOutputPort());

    // Visualize
    ImageRenderer::pointer imageRenderer = ImageRenderer::New();
    imageRenderer->addInputConnection(cropper->getOutputPort());
    TriangleRenderer::pointer TriangleRenderer = TriangleRenderer::New();
    TriangleRenderer->addInputConnection(detection->getOutputPort());

    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(imageRenderer);
    window->addRenderer(TriangleRenderer);
    window->set2DMode();
    window->setWidth(1920);
    window->setHeight(1080);
    window->enableFullscreen();
    window->start();
}
