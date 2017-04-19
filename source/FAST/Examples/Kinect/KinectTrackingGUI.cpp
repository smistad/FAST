#include "KinectTrackingGUI.hpp"
#include "KinectTracking.hpp"
#include "FAST/Streamers/KinectStreamer.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include <QVBoxLayout>
#include <FAST/Visualization/SegmentationRenderer/SegmentationRenderer.hpp>

namespace fast {

class MouseListener : public QObject {
    public:
        MouseListener(KinectTracking::pointer tracking, View* view);
    protected:
        bool eventFilter(QObject *obj, QEvent *event);
    private:
        KinectTracking::pointer mTracking;
        Vector2i mPreviousMousePosition;
        View* mView;
};

MouseListener::MouseListener(KinectTracking::pointer tracking, View* view) : QObject(view) {
    mTracking = tracking;
    mPreviousMousePosition = Vector2i(-1, -1);
    mView = view;
}

bool MouseListener::eventFilter(QObject *obj, QEvent *event) {
    if(event->type() == QEvent::MouseMove) {
        // Releay mouse movement to tracking
        QMouseEvent *keyEvent = static_cast<QMouseEvent *>(event);
        float spacing = mView->get2DPixelSpacing();
        if(mPreviousMousePosition.x() == -1 && mPreviousMousePosition.y() == -1) {
            mPreviousMousePosition = Vector2i(keyEvent->x()*spacing, keyEvent->y()*spacing);
        } else {
            Vector2i current(keyEvent->x()*spacing, keyEvent->y()*spacing);
            mTracking->addLine(mPreviousMousePosition, current);
            mPreviousMousePosition = current;
        }
        return true;
    } else {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}

KinectTrackingGUI::KinectTrackingGUI() {
    View* view = createView();

    // Setup streaming
    KinectStreamer::pointer streamer = KinectStreamer::New();

    // Tracking
    KinectTracking::pointer tracking = KinectTracking::New();
    tracking->setInputConnection(streamer->getOutputPort(0));

    // Set up mouse listener
    view->installEventFilter(new MouseListener(tracking, view));

    // Renderer RGB image
    ImageRenderer::pointer renderer = ImageRenderer::New();
    renderer->addInputConnection(tracking->getOutputPort(0));

    SegmentationRenderer::pointer annotationRenderer = SegmentationRenderer::New();
    annotationRenderer->addInputConnection(tracking->getOutputPort(1));
    annotationRenderer->setFillArea(false);

    view->set2DMode();
    view->setBackgroundColor(Color::Black());
    view->addRenderer(renderer);
    view->addRenderer(annotationRenderer);
    setWidth(1280);
    setHeight(768);
    setTitle("FAST - Kinect Object Tracking");

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(view);
    mWidget->setLayout(layout);
}

}