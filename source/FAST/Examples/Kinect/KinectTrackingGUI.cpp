#include "KinectTrackingGUI.hpp"
#include "KinectTracking.hpp"
#include "FAST/Streamers/KinectStreamer.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QDir>
#include <QElapsedTimer>
#include <QListWidget>
#include <QDirIterator>
#include <QMessageBox>
#include <FAST/Visualization/SegmentationRenderer/SegmentationRenderer.hpp>
#include <FAST/Visualization/PointRenderer/PointRenderer.hpp>
#include <FAST/Streamers/MeshFileStreamer.hpp>

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

    mRecordTimer = new QElapsedTimer;

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
    const int menuWidth = 300;

    view->set2DMode();
    view->setBackgroundColor(Color::Black());
    view->addRenderer(renderer);
    view->addRenderer(annotationRenderer);
    setWidth(1024 + menuWidth);
    setHeight(848);
    setTitle("FAST - Kinect Object Tracking");

    QVBoxLayout* menuLayout = new QVBoxLayout;
    menuLayout->setAlignment(Qt::AlignTop);

    // Logo
    QImage* image = new QImage;
    image->load((Config::getDocumentationPath() + "images/FAST_logo_square.png").c_str());
    QLabel* logo = new QLabel;
    logo->setPixmap(QPixmap::fromImage(image->scaled(menuWidth, ((float)menuWidth/image->width())*image->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
    logo->adjustSize();
    menuLayout->addWidget(logo);

    // Title label
    QLabel* title = new QLabel;
    title->setText("<div style=\"text-align: center; font-weight: bold; font-size: 24px;\">Kinect<br>Object Tracking</div>");
    menuLayout->addWidget(title);

    // Quit button
    QPushButton* quitButton = new QPushButton;
    quitButton->setText("Quit (q)");
    quitButton->setStyleSheet("QPushButton { background-color: red; color: white; }");
    quitButton->setFixedWidth(menuWidth);
    menuLayout->addWidget(quitButton);

    QPushButton* restartButton = new QPushButton;
    restartButton->setText("Restart");
    restartButton->setStyleSheet("QPushButton { background-color: green; color: white; }");
    QObject::connect(restartButton, &QPushButton::clicked, std::bind(&KinectTrackingGUI::restart, this));
    menuLayout->addWidget(restartButton);

    QLabel* storageDirLabel = new QLabel;
    storageDirLabel->setText("Storage directory");
    menuLayout->addWidget(storageDirLabel);

    mStorageDir = new QLineEdit;
    mStorageDir->setText(QDir::homePath() + QDir::separator() + QString("FAST_Kinect_Recordings"));
    mStorageDir->setFixedWidth(menuWidth);
    menuLayout->addWidget(mStorageDir);

    mRecordButton = new QPushButton;
    mRecordButton->setText("Record");
    mRecordButton->setStyleSheet("QPushButton { background-color: green; color: white; }");
    QObject::connect(mRecordButton, &QPushButton::clicked, std::bind(&KinectTrackingGUI::toggleRecord, this));
    menuLayout->addWidget(mRecordButton);

    mRecordingInformation = new QLabel;
    mRecordingInformation->setFixedWidth(menuWidth);
    mRecordingInformation->setStyleSheet("QLabel { font-size: 14px; }");
    menuLayout->addWidget(mRecordingInformation);

    mRecordingsList = new QListWidget;
    menuLayout->addWidget(mRecordingsList);
    mRecordingsList->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    mRecordingsList->setFixedHeight(100);
    refreshRecordingsList();

    QPushButton* playButton = new QPushButton;
    playButton->setText("Play");
    playButton->setStyleSheet("QPushButton { background-color: green; color: white; }");
    menuLayout->addWidget(playButton);
    QObject::connect(playButton, &QPushButton::clicked, std::bind(&KinectTrackingGUI::playRecording, this));

    QHBoxLayout* layout = new QHBoxLayout;
    layout->addLayout(menuLayout);
    layout->addWidget(view);
    mWidget->setLayout(layout);

    // Update messages frequently
    QTimer* timer = new QTimer(this);
    timer->start(1000/5); // in milliseconds
    timer->setSingleShot(false);
    QObject::connect(timer, &QTimer::timeout, std::bind(&KinectTrackingGUI::updateMessages, this));
}

void KinectTrackingGUI::playRecording() {
    auto selectedItems = mRecordingsList->selectedItems();
    if(selectedItems.size() == 0) {
        // Show error message
        QMessageBox* message = new QMessageBox;
        message->setWindowTitle("Error");
        message->setText("You did not select a recording.");
        message->show();
        return;
    }
    std::string selectedRecording = (
            mStorageDir->text() +
            QDir::separator() +
            selectedItems[0]->text() +
            QDir::separator()
        ).toStdString();
    selectedRecording += "#.vtk";
    std::cout << selectedRecording << std::endl;

    stopComputationThread();
    getView(0)->removeAllRenderers();

    MeshFileStreamer::pointer streamer = MeshFileStreamer::New();
    streamer->setFilenameFormat(selectedRecording);
    streamer->enableLooping();

    PointRenderer::pointer cloudRenderer = PointRenderer::New();
    cloudRenderer->setDefaultSize(1);
    cloudRenderer->addInputConnection(streamer->getOutputPort());

    getView(0)->set3DMode();
    getView(0)->addRenderer(cloudRenderer);
    getView(0)->setLookAt(Vector3f(0,-500,-500), Vector3f(0,0,1000), Vector3f(0,-1,0), 500, 5000);
    getView(0)->reinitialize();

    startComputationThread();
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

void KinectTrackingGUI::toggleRecord() {
    bool recording = mTracking->toggleRecord(mStorageDir->text().toStdString());
    if(recording) {
        mRecordButton->setText("Stop recording");
        mRecordButton->setStyleSheet("QPushButton { background-color: red; color: white; }");
        mStorageDir->setDisabled(true);
        mRecordTimer->start();
        std::string msg = "Recording to: " + mTracking->getRecordingName();
        mRecordingInformation->setText(msg.c_str());
    } else {
        mRecordButton->setText("Record");
        mRecordButton->setStyleSheet("QPushButton { background-color: green; color: white; }");
        mStorageDir->setDisabled(false);
    }
}

void KinectTrackingGUI::updateMessages() {
    if(mTracking->isRecording()) {
        std::string msg = "Recording to: " + mTracking->getRecordingName() + "\n";
        msg += std::to_string(mTracking->getFramesStored()) + " frames stored\n";
        msg += format("%.1f seconds", (float)mRecordTimer->elapsed()/1000.0f);
        mRecordingInformation->setText(msg.c_str());
    }
}

void KinectTrackingGUI::refreshRecordingsList() {
    // Get all folders in the folder mStorageDir
    QDirIterator it(mStorageDir->text());
    mRecordingsList->clear();
    while(it.hasNext()) {
        it.next();
        QString next = it.fileName();
        if(next.size() > 4)
            mRecordingsList->addItem(next);
    }
}

}