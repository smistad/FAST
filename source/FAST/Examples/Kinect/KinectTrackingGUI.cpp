#include "KinectTrackingGUI.hpp"
#include "FAST/Streamers/KinectStreamer.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include <QVBoxLayout>

namespace fast {

KinectTrackingGUI::KinectTrackingGUI() {
    View* view = createView();

    // Setup streaming
    KinectStreamer::pointer streamer = KinectStreamer::New();

    // Renderer RGB image
    ImageRenderer::pointer renderer = ImageRenderer::New();
    renderer->addInputConnection(streamer->getOutputPort(0));

    view->set2DMode();
    view->setBackgroundColor(Color::Black());
    view->addRenderer(renderer);
    setWidth(1280);
    setHeight(768);
    setTitle("FAST - Kinect Object Tracking");

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(view);
    mWidget->setLayout(layout);
}

}