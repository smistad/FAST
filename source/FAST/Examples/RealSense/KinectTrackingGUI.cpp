#include "KinectTrackingGUI.hpp"
#include "KinectTracking.hpp"
#include "FAST/Streamers/RealSenseStreamer.hpp"
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
#include <FAST/Visualization/VertexRenderer/VertexRenderer.hpp>
#include <FAST/Streamers/MeshFileStreamer.hpp>
#include <FAST/Exporters/VTKMeshFileExporter.hpp>
#include <FAST/Importers/VTKMeshFileImporter.hpp>
#include <QProgressDialog>

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
        // TODO need to go from view coordinates to physical to image coordinates
        const Matrix4f perspectiveMatrix = mView->getPerspectiveMatrix();
        const Matrix4f viewMatrix = mView->getViewMatrix();
        Vector3f current(2.0f*(float)mouseEvent->x()/mView->width() - 1.0f, -(2.0f*(float)mouseEvent->y()/mView->height() -1.0f), 0);
        current = (viewMatrix.inverse()*perspectiveMatrix.inverse()*current.homogeneous()).head(3);
        if(mPreviousMousePosition.x() == -1 && mPreviousMousePosition.y() == -1) {
            mPreviousMousePosition = current.cast<int>().head(2);
        } else {
            mTracking->addLine(mPreviousMousePosition, current.cast<int>().head(2));
            mPreviousMousePosition = current.cast<int>().head(2);
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
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if(keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return) {
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
    mStreamer = RealSenseStreamer::New();
    mStreamer->getReporter().setReportMethod(Reporter::COUT);
    mStreamer->setMaxRange(3*1000); // All points above x meters are excluded

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

    setTitle("FAST - Kinect Object Tracking");
    setWidth(1024 + menuWidth);
    setHeight(848);
    enableMaximized();
    view->set2DMode();
    view->setBackgroundColor(Color::Black());
    view->addRenderer(renderer);
    view->addRenderer(annotationRenderer);

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

    // Quit butto1.0f;//n
    QPushButton* quitButton = new QPushButton;
    quitButton->setText("Quit (q)");
    quitButton->setStyleSheet("QPushButton { background-color: red; color: white; }");
    quitButton->setFixedWidth(menuWidth);
    QObject::connect(quitButton, &QPushButton::clicked, std::bind(&Window::stop, this));
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

    QLabel* recordingNameLabel = new QLabel;
    recordingNameLabel->setText("Recording name");
    menuLayout->addWidget(recordingNameLabel);

    mRecordingNameLineEdit = new QLineEdit;
    mRecordingNameLineEdit->setFixedWidth(menuWidth);
    menuLayout->addWidget(mRecordingNameLineEdit);

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
    mRecordingsList->setSortingEnabled(true);
    refreshRecordingsList();

    mPlayButton = new QPushButton;
    mPlayButton->setText("Play");
    mPlayButton->setStyleSheet("QPushButton { background-color: green; color: white; }");
    menuLayout->addWidget(mPlayButton);
    QObject::connect(mPlayButton, &QPushButton::clicked, std::bind(&KinectTrackingGUI::playRecording, this));

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
    mPlaying = !mPlaying;
    if(!mPlaying) {
        mPlayButton->setText("Play");
        mPlayButton->setStyleSheet("QPushButton { background-color: green; color: white; }");
        restart();
    } else {
        auto selectedItems = mRecordingsList->selectedItems();
        if(selectedItems.size() == 0) {
            // Show error message
            QMessageBox *message = new QMessageBox;
            message->setWindowTitle("Error");
            message->setText("You did not select a recording.");
            message->show();
            return;
        }
        mStreamer->stop();
        std::string selectedRecording = (
                mStorageDir->text() +
                QDir::separator() +
                selectedItems[0]->text() +
                QDir::separator()
        ).toUtf8().constData();


        // Set up streaming from disk
        auto streamer = MeshFileStreamer::New();
        streamer->setFilenameFormat(selectedRecording + "#.vtk");

        // Get the number of files
        QDirIterator it(selectedRecording.c_str());
        int numFiles = 0;
        while(it.hasNext()) {
            it.next();
            if(it.fileName().size() > 4 && it.fileName() != "target.vtk")
                numFiles++;
        }
        streamer->setMaximumNumberOfFrames(numFiles);
        streamer->update(0); // start loading

        QProgressDialog progress("Loading recording ...", "Abort", 0, numFiles, mWidget);
        progress.setWindowTitle("Loading");
        progress.setWindowModality(Qt::WindowModal);
        progress.show();

        while(streamer->getNrOfFrames() != numFiles) {
            progress.setValue(streamer->getNrOfFrames());
            if(progress.wasCanceled()) {
                streamer->stop();
                restart();
                return;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        progress.setValue(numFiles);

        stopComputationThread();
        getView(0)->removeAllRenderers();

        // Load target cloud
        auto importer = VTKMeshFileImporter::New();
        importer->setFilename(selectedRecording + "target.vtk");
        auto port = importer->getOutputPort();
        importer->update(0);
        auto targetCloud = port->getNextFrame<Mesh>();

        mTracking->setInputConnection(1, streamer->getOutputPort());
        mTracking->setTargetCloud(targetCloud);

        auto cloudRenderer = VertexRenderer::New();
        cloudRenderer->setDefaultSize(1.5);
        cloudRenderer->addInputConnection(mTracking->getOutputPort(2));
        cloudRenderer->addInputConnection(streamer->getOutputPort(0));
        cloudRenderer->setColor(0, Color::Green());

        getView(0)->set3DMode();
        getView(0)->addRenderer(cloudRenderer);
        getView(0)->setLookAt(Vector3f(0, -500, -500), Vector3f(0, 0, 1000), Vector3f(0, -1, 0), 500, 5000);
        getView(0)->reinitialize();

        startComputationThread();
        mPlayButton->setText("Stop");
        mPlayButton->setStyleSheet("QPushButton { background-color: red; color: white; }");
    }
}



void KinectTrackingGUI::extractPointCloud() {
    stopComputationThread();
    getView(0)->removeAllRenderers();

    mTracking->calculateTargetCloud(mStreamer);

    // If recording is enabled: Store the target cloud, then activate recording on tracking object
    if(mRecording) {
        // Create recording path
        std::string path = mStorageDir->text().toUtf8().constData();
        if(mRecordingNameLineEdit->text() != "") {
            mRecordingName =  currentDateTime() + " " + mRecordingNameLineEdit->text().toUtf8().constData();
        } else {
            mRecordingName = currentDateTime();
        }
        std::string recordingPath = (QString(path.c_str()) + QDir::separator() + QString(mRecordingName.c_str()) + QDir::separator()).toUtf8().constData();
        createDirectories(recordingPath);

        // Store target cloud
        VTKMeshFileExporter::pointer exporter = VTKMeshFileExporter::New();
        exporter->setInputData(mTracking->getTargetCloud());
        exporter->setFilename(recordingPath + "target.vtk");
        exporter->update(0);

        // Start saving point clouds
        mTracking->startRecording(recordingPath);
    }

    // Reset ports
    mTracking->setInputConnection(0, mStreamer->getOutputPort(0));
    mTracking->setInputConnection(1, mStreamer->getOutputPort(2));
    auto cloudRenderer = VertexRenderer::New();
    cloudRenderer->setDefaultSize(1.5);
    uint port = cloudRenderer->addInputConnection(mTracking->getOutputPort(2));
    cloudRenderer->setColor(port, Color::Green());
    cloudRenderer->addInputConnection(mStreamer->getOutputPort(2));

    getView(0)->set3DMode();
    getView(0)->addRenderer(cloudRenderer);
    getView(0)->setLookAt(Vector3f(0,-500,-500), Vector3f(0,0,1000), Vector3f(0,-1,0), 500, 5000);
    getView(0)->reinitialize(); // Reinitialize runs update call, cannot be called after startComputationThread
    startComputationThread();
}

void KinectTrackingGUI::restart() {
    View* view = getView(0);
    stopComputationThread();
    view->removeAllRenderers();

    // Setup streaming
    mStreamer = RealSenseStreamer::New();
    mStreamer->getReporter().setReportMethod(Reporter::COUT);
    mStreamer->setMaxRange(3*1000); // All points above x meters are excluded

    mTracking->setInputConnection(0, mStreamer->getOutputPort(0));
    mTracking->setInputConnection(1, mStreamer->getOutputPort(2));
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
    mRecording = !mRecording;
    if(mRecording) {
        mRecordButton->setText("Stop recording");
        mRecordButton->setStyleSheet("QPushButton { background-color: red; color: white; }");
        mStorageDir->setDisabled(true);
        mRecordTimer->start();
    } else {
        mRecordButton->setText("Record");
        mRecordButton->setStyleSheet("QPushButton { background-color: green; color: white; }");
        mStorageDir->setDisabled(false);
        mTracking->stopRecording();
        refreshRecordingsList();
    }
}

void KinectTrackingGUI::updateMessages() {
    if(mTracking->isRecording()) {
        std::string msg = "Recording to: " + mRecordingName + "\n";
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