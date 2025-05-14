#include "CameraStreamer.hpp"
#include <FAST/Data/Image.hpp>
#include <QCamera>
#include <QMediaCaptureSession>
#include <FAST/Visualization/Window.hpp>
#include <QMediaDevices>
#include <QVideoSink>

namespace fast {

CameraStreamer::CameraStreamer(bool grayscale, uchar cameraIndex) {
    createOutputPort<Image>(0);

    setGrayscale(grayscale);
    setCamera(cameraIndex);

    createBooleanAttribute("grayscale", "Grayscale", "Convert camera to grayscale while streaming", grayscale);
    createIntegerAttribute("camera", "Camera Index", "Select which camera to use", cameraIndex);
    setStreamingMode(StreamingMode::NewestFrameOnly);
    setModified(true);
}

void CameraStreamer::loadAttributes() {
    setGrayscale(getBooleanAttribute("grayscale"));
    setCamera(getIntegerAttribute("camera"));
}

void CameraStreamer::setCamera(uchar index) {
    m_cameraIndex = index;
}

void CameraStreamer::execute() {
    if(!m_streamIsStarted)
        Window::initializeQtApp();
    startStream();

    waitForFirstFrame();
    reportInfo() << "Finished camera streamer execute" << reportEnd();
}

void CameraStreamer::addNewImageFrame(const uchar* data, int width, int height) {
    auto output = Image::create(
        width,
        height,
        TYPE_UINT8,
        mGrayscale ? 1 : 3,
        getMainDevice(),
        data
    );
    std::chrono::duration<double, std::milli> elapsed = std::chrono::high_resolution_clock::now() - m_startTime;
    output->setCreationTimestamp((uint64_t)elapsed.count());
    addOutputData(0, output);
    ++m_framesAdded;
    Reporter::info() << "Added frames: " << m_framesAdded << Reporter::end();
    // Make sure we end the waiting thread if first frame has not been inserted
    frameAdded();
}

void CameraStreamer::setFinished(bool finished) {
    m_finished = finished;
}

bool CameraStreamer::getGrayscale() const {
    return mGrayscale;
}

void CameraStreamer::setGrayscale(bool grayscale) {
    mGrayscale = grayscale;
}

void CameraStreamer::generateStream() {
    // Get camera
    Reporter::info() << "Getting cameras.." << Reporter::end();
    auto cameras = QMediaDevices::videoInputs();
    if(cameras.empty())
        throw Exception("No camera was found on the system.");
    if(m_cameraIndex >= cameras.size())
        throw Exception("Camera index " + std::to_string(m_cameraIndex) + " is larger than nr of cameras");
    for(auto& cameraInfo : cameras) {
        Reporter::info() << "Camera: " << cameraInfo.description().toStdString() << Reporter::end();
    }
    // Using separate QThread and Worker object so that slots are invoked in the new thread
    //thread = new QThread; // TODO Is this really needed when we have a event loop in generateStream?
    auto worker = new CameraWorker(new QCamera(cameras[m_cameraIndex]));
    //worker->moveToThread(thread);
    //QObject::connect(worker, SIGNAL(error(QString)), this, SLOT(errorString(QString)));
    //QObject::connect(thread, &QThread::started, worker, &CameraWorker::run);
    //QObject::connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
    //QObject::connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
    //QObject::connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    //QObject::connect(thread, SIGNAL(finished()), worker, SLOT(deleteLater()));
    QObject::connect(worker, &CameraWorker::newFrame, [this](const QVideoFrame& frame) {
        if(frame.isValid()) {
            const int width = frame.width();
            const int height = frame.height();
            QImage image = frame.toImage();
#ifdef WIN32
            image = image.flipped(Qt::Horizontal | Qt::Vertical);
#else
            image = image.flipped(Qt::Horizontal);
#endif

            QImage::Format format;
            if(getGrayscale()) {
                format = QImage::Format_Grayscale8;
            } else {
                format = QImage::Format_RGB888;
            }
            QImage convertedImage = image.convertToFormat(format);
            try {
                addNewImageFrame(convertedImage.constBits(), width, height);
                Reporter::info() << "Finished processing camera frame" << Reporter::end();
            } catch(ThreadStopped & e) {
            }
        }
    });
    //auto image = Image::create(512, 512, TYPE_UINT8, 3);
    //image->fill(0);
    //addOutputData(0, image);
    //frameAdded();
    m_startTime = std::chrono::high_resolution_clock::now();
    //thread->start();
    // We need an event loop
    m_eventLoop = new QEventLoop;
    // Call worker run on startup
    auto timer = new QTimer;
    timer->setInterval(1);
    timer->setSingleShot(true);
    timer->start();
    QObject::connect(timer, &QTimer::timeout, [worker]() {
        worker->run();
    });
    //QObject::connect(thread, &QThread::finished, loop, &QEventLoop::quit);
    m_eventLoop->exec();
    Reporter::info() << "Exited event loop of CameraStreamer" << Reporter::end();
    delete worker;
    delete m_eventLoop;
    delete timer;
    m_eventLoop = nullptr;
    m_finished = true;
}

void CameraStreamer::stop() {
   reportInfo() << "Camera streamer stopped called." << reportEnd();
   //thread->quit();
   if(m_eventLoop != nullptr)
       m_eventLoop->quit();
   Streamer::stop();
}

CameraStreamer::~CameraStreamer() {
    reportInfo() << "Destroying camera streamer.." << reportEnd();
    stop();
    reportInfo() << "Camera streamer destroyed." << reportEnd();
}

CameraWorker::CameraWorker(QCamera* camera) {
    m_camera = camera;
}

CameraWorker::~CameraWorker() {
    Reporter::info() << "Destroying camera worker.." << Reporter::end();
    m_camera->stop();
    delete m_captureSession;
    delete m_camera;
    delete m_mySink;
    Reporter::info() << "Camera worker destroyed." << Reporter::end();
}

void CameraWorker::run() {
    m_captureSession = new QMediaCaptureSession(); // TODO cleanup this
    m_mySink = new QVideoSink();
    Reporter::info() << "Starting camera.." << Reporter::end();
    m_captureSession->setCamera(m_camera);
    m_captureSession->setVideoSink(m_mySink);
    connect(m_mySink, &QVideoSink::videoFrameChanged, [this](const QVideoFrame& frame) {
        emit newFrame(frame);
    });
	m_camera->start();
	if(m_camera->error() != QCamera::NoError) {
	    Reporter::error() << "Camera error: " << m_camera->errorString().toStdString() << Reporter::end();
	} else {
	    Reporter::info() << "Camera started." << Reporter::end();
	}
}



}
