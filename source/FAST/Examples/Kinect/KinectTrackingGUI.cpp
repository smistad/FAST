#include "KinectTrackingGUI.hpp"
#include "KinectTracking.hpp"
#include "FAST/Streamers/KinectStreamer.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include <QVBoxLayout>
#include <QPushButton>
#include <FAST/Visualization/SegmentationRenderer/SegmentationRenderer.hpp>
#include <FAST/Visualization/PointRenderer/PointRenderer.hpp>

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

class KeyPressListener : public QObject {
    public:
        KeyPressListener(KinectTrackingGUI* gui, QWidget* widget);
    protected:
        bool eventFilter(QObject *obj, QEvent *event);
    private:
        KinectTrackingGUI* mGUI;
};

MouseListener::MouseListener(KinectTracking::pointer tracking, View* view) : QObject(view) {
    mTracking = tracking;
    mPreviousMousePosition = Vector2i(-1, -1);
    mView = view;
}

bool MouseListener::eventFilter(QObject *obj, QEvent *event) {
    if(event->type() == QEvent::MouseButtonRelease) {
        mPreviousMousePosition = Vector2i(-1, -1);
    }
    if(event->type() == QEvent::MouseMove) {
        // Releay mouse movement to tracking
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        float spacing = mView->get2DPixelSpacing();
        if(mPreviousMousePosition.x() == -1 && mPreviousMousePosition.y() == -1) {
            mPreviousMousePosition = Vector2i(mouseEvent->x() * spacing, mouseEvent->y() * spacing);
        } else {
            Vector2i current(mouseEvent->x() * spacing, mouseEvent->y() * spacing);
            mTracking->addLine(mPreviousMousePosition, current);
            mPreviousMousePosition = current;
        }
    }

    // standard event processing
    return QObject::eventFilter(obj, event);
}


KeyPressListener::KeyPressListener(KinectTrackingGUI* gui, QWidget* parent) : QObject(parent) {
    mGUI = gui;
}

bool KeyPressListener::eventFilter(QObject *obj, QEvent *event) {
    if(event->type() == QEvent::KeyPress) {
        std::cout << "Key event" << std::endl;
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if(keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return) {
            std::cout << "Extracting target cloud!" << std::endl;
            mGUI->extractPointCloud();
            return true;
        } else {
            return QObject::eventFilter(obj, event);
        }
    } else {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}

KinectTrackingGUI::KinectTrackingGUI() {
    View* view = createView();

    // Setup streaming
    mStreamer = KinectStreamer::New();
    mStreamer->setPointCloudFiltering(true);

    // Tracking
    mTracking = KinectTracking::New();
    mTracking->setInputConnection(0, mStreamer->getOutputPort(0));
    mTracking->setInputConnection(1, mStreamer->getOutputPort(2));

    // Set up mouse and key listener
    view->installEventFilter(new MouseListener(mTracking, view));
    mWidget->installEventFilter(new KeyPressListener(this, mWidget));

    // Renderer RGB image
    ImageRenderer::pointer renderer = ImageRenderer::New();
    renderer->addInputConnection(mTracking->getOutputPort(0));

    SegmentationRenderer::pointer annotationRenderer = SegmentationRenderer::New();
    annotationRenderer->addInputConnection(mTracking->getOutputPort(1));
    annotationRenderer->setFillArea(false);

    view->set2DMode();
    view->setBackgroundColor(Color::Black());
    view->addRenderer(renderer);
    view->addRenderer(annotationRenderer);
    setWidth(1024);
    setHeight(848);
    setTitle("FAST - Kinect Object Tracking");

    QVBoxLayout* layout = new QVBoxLayout;

    QPushButton* restartButton = new QPushButton;
    restartButton->setText("Restart");
    restartButton->setStyleSheet("QPushButton { background-color: green; font-size: 24px; color: white; }");
    QObject::connect(restartButton, &QPushButton::clicked, std::bind(&KinectTrackingGUI::restart, this));

    layout->addWidget(restartButton);
    layout->addWidget(view);
    mWidget->setLayout(layout);
}

void KinectTrackingGUI::extractPointCloud() {
    stopComputationThread();
    getView(0)->removeAllRenderers();

    PointRenderer::pointer cloudRenderer = PointRenderer::New();
    cloudRenderer->setDefaultSize(1.5);
    mTracking->calculateTargetCloud(mStreamer);
    cloudRenderer->addInputConnection(mTracking->getOutputPort(2));
    cloudRenderer->addInputConnection(mStreamer->getOutputPort(2));
    cloudRenderer->setColor(mTracking->getOutputPort(2), Color::Green());

    getView(0)->set3DMode();
    getView(0)->addRenderer(cloudRenderer);
    getView(0)->setLookAt(Vector3f(0,-500,-500), Vector3f(0,0,1000), Vector3f(0,-1,0), 500, 5000);
    getView(0)->reinitialize();

    startComputationThread();
}

void KinectTrackingGUI::restart() {
    View* view = getView(0);
    stopComputationThread();
    view->removeAllRenderers();

    mTracking->restart();

    // Renderer RGB image
    ImageRenderer::pointer renderer = ImageRenderer::New();
    renderer->addInputConnection(mTracking->getOutputPort(0));

    SegmentationRenderer::pointer annotationRenderer = SegmentationRenderer::New();
    annotationRenderer->addInputConnection(mTracking->getOutputPort(1));
    annotationRenderer->setFillArea(false);

    view->set2DMode();
    view->setBackgroundColor(Color::Black());
    view->addRenderer(renderer);
    view->addRenderer(annotationRenderer);
    view->reinitialize();

    startComputationThread();
}

}