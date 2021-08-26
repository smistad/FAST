#include "CameraStreamer.hpp"
#include <FAST/Data/Image.hpp>
#include <QtMultimedia/QAbstractVideoSurface>
#include <QtMultimedia/QCameraViewfinderSettings>
#include <QtMultimedia/QCamera>
#include <FAST/Visualization/Window.hpp>

namespace fast {

class CameraVideoSurface : public QAbstractVideoSurface {

public:
    CameraStreamer* streamer;

    QList<QVideoFrame::PixelFormat> supportedPixelFormats(
        QAbstractVideoBuffer::HandleType handleType = QAbstractVideoBuffer::NoHandle) const {
        Q_UNUSED(handleType);

        // Return the formats you will support
        return QList<QVideoFrame::PixelFormat>()
            << QVideoFrame::Format_RGB565
            << QVideoFrame::Format_RGB24
            << QVideoFrame::Format_RGB32
            << QVideoFrame::Format_RGB555;
    }

    bool present(const QVideoFrame& frame) {
        Reporter::info() << "Frame received!" << Reporter::end();
        Reporter::info() << "Format: " << frame.pixelFormat() << " " << frame.width() << " " << frame.height() << Reporter::end();
        if(!frame.isValid())
            return false;
        if(frame.pixelFormat() == QVideoFrame::PixelFormat::Format_Invalid) {
            Reporter::info() << "Camera Finished." << Reporter::end();
            streamer->setFinished(true);
            return false;
        }

        QVideoFrame cloneFrame(frame);
        cloneFrame.map(QAbstractVideoBuffer::ReadOnly);
        const int width = frame.width();
        const int height = frame.height();
        QImage image(cloneFrame.bits(), width, height, frame.bytesPerLine(), QVideoFrame::imageFormatFromPixelFormat(frame.pixelFormat()));
#ifdef WIN32
        image = image.mirrored(true, true);
#else
        image = image.mirrored(true, false);
#endif

        QImage::Format format;
        if(streamer->getGrayscale()) {
            format = QImage::Format_Grayscale8;
        } else {
            format = QImage::Format_RGB888;
        }
        QImage convertedImage = image.convertToFormat(format);
        try {
            streamer->addNewImageFrame(convertedImage.constBits(), width, height);
            Reporter::info() << "Finished processing camera frame" << Reporter::end();
        } catch(ThreadStopped & e) {
        }

        return true;
    }

    void stateChanged(QCamera::State state) {
		Reporter::info() << "QCamera state changed" << Reporter::end();
    }
};

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
    if(!m_streamIsStarted) {
		// Get camera
		Reporter::info() << "Getting cameras.." << Reporter::end();
		auto cameras = QCameraInfo::availableCameras();
        if(cameras.empty())
            throw Exception("No camera was found on the system.");
        if(m_cameraIndex >= cameras.size())
            throw Exception("Camera index " + std::to_string(m_cameraIndex) + " is larger than nr of cameras");
		for(auto& cameraInfo : cameras) {
			Reporter::info() << "Camera: " << cameraInfo.deviceName().toStdString() << Reporter::end();
		}
		thread = new QThread;
        auto worker = new CameraWorker(this, cameras[m_cameraIndex]);
        worker->moveToThread(thread);
        //QObject::connect(worker, SIGNAL(error(QString)), this, SLOT(errorString(QString)));
        QObject::connect(thread, SIGNAL(started()), worker, SLOT(run()));
        QObject::connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
        QObject::connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
        QObject::connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
        QObject::connect(thread, SIGNAL(finished()), worker, SLOT(deleteLater()));
        m_startTime = std::chrono::high_resolution_clock::now();
        thread->start();
        m_streamIsStarted = true;
    }

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

CameraWorker::CameraWorker(CameraStreamer* streamer, const QCameraInfo cameraInfo) {
    mStreamer = streamer;
    m_cameraInfo = cameraInfo;
}

CameraWorker::~CameraWorker() {
}

void CameraWorker::run() {
    m_camera = std::make_unique<QCamera>(m_cameraInfo);
	auto myVideoSurface = new CameraVideoSurface;
	myVideoSurface->streamer = mStreamer;
    QObject::connect(m_camera.get(), &QCamera::stateChanged, std::bind(&CameraVideoSurface::stateChanged, myVideoSurface, std::placeholders::_1));
	m_camera->setViewfinder(myVideoSurface);
    Reporter::info() << "Starting camera.." << Reporter::end();
	m_camera->start();
    Reporter::info() << "Started" << Reporter::end();
}



}
