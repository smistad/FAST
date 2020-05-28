#include "MovieStreamer.hpp"
#include <QtMultimedia/QAbstractVideoSurface>
#include <QtMultimedia/QMediaPlayer>
#include <QtMultimediaWidgets/QVideoWidget>
#include <QApplication>
#include <QThread>
#include <FAST/Data/Image.hpp>

namespace fast {

class VideoSurface : public QAbstractVideoSurface {

    public:
        MovieStreamer* streamer;

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

    bool present(const QVideoFrame &frame) {
        Reporter::info() << "Movie frame received!" << Reporter::end();
        Reporter::info() << "Format: " << frame.pixelFormat() << Reporter::end();
        if(frame.pixelFormat() == QVideoFrame::PixelFormat::Format_Invalid) {
            // Movie is finished
            Reporter::info() << "Movie finished." << Reporter::end();
            streamer->setFinished(true);
            return false;
        }

        QVideoFrame cloneFrame(frame);
        cloneFrame.map(QAbstractVideoBuffer::ReadOnly);
        const int width = frame.width();
        const int height = frame.height();
        QImage image(cloneFrame.bits(), width, height, frame.bytesPerLine(), QVideoFrame::imageFormatFromPixelFormat(frame.pixelFormat()));

        QImage::Format format;
        if(streamer->getGrayscale()) {
            format = QImage::Format_Grayscale8;
        } else {
            format = QImage::Format_RGB888;
        }
        QImage convertedImage = image.convertToFormat(format);
        try {
            streamer->addNewImageFrame(convertedImage.constBits(), width, height);
            Reporter::info() << "Finished processing movie frame" << Reporter::end();
        } catch(ThreadStopped &e) {
        }

        return true;
    }

    void stateChanged(QMediaPlayer::State state) {
        if(state == QMediaPlayer::StoppedState) {
            Reporter::info() << "QMediaPlayer state changed to stopped - stopping stream" << Reporter::end();
            streamer->setFinished(true);
        }
    }
};

void MovieStreamer::addNewImageFrame(const uchar* data, int width, int height) {
    Image::pointer output = Image::New();
    output->create(
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

void MovieStreamer::setFilename(std::string filename) {
    mFilename = filename;
    mIsModified = true;
}

void MovieStreamer::loadAttributes() {
    setFilename(getStringAttribute("filename"));
    setGrayscale(getBooleanAttribute("grayscale"));
}

MovieStreamer::MovieStreamer() {
    createOutputPort<Image>(0);

    createStringAttribute("filename", "Filename", "Filepath to movie file", "");
    createBooleanAttribute("grayscale", "Grayscale", "Convert movie to grayscale while streaming", mGrayscale);
}

Worker::Worker(MovieStreamer* streamer) {
    mStreamer = streamer;
}

Worker::~Worker() {
}

void Worker::run() {
    m_player = std::make_unique<QMediaPlayer>();
    //std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    VideoSurface* myVideoSurface = new VideoSurface;
    myVideoSurface->streamer = mStreamer;
    m_player->setMuted(true);
    m_player->setVideoOutput(myVideoSurface);
    QObject::connect(m_player.get(), &QMediaPlayer::stateChanged, std::bind(&VideoSurface::stateChanged, myVideoSurface, std::placeholders::_1));
    std::string prefix = "";
    if(mStreamer->getFilename()[0] != '/' && mStreamer->getFilename()[1] == ':') // On windows w want file:///C:/asd/asd..
        prefix = "/";
    m_player->setMedia(QUrl(("file://" + prefix + mStreamer->getFilename()).c_str()));
    m_player->play();
    Reporter::info() << "Play returned" << Reporter::end();
    //emit finished();
}


void MovieStreamer::execute() {
    if(!m_streamIsStarted) {
        if(!fileExists(mFilename))
            throw FileNotFoundException(mFilename);
        thread = new QThread;
        Worker* worker = new Worker(this);
        worker->moveToThread(thread);
        //QObject::connect(worker, SIGNAL(error(QString)), this, SLOT(errorString(QString)));
        QObject::connect(thread, SIGNAL(started()), worker, SLOT(run()));
        QObject::connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
        QObject::connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
        QObject::connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
        QObject::connect(thread, SIGNAL(finished()), worker, SLOT(deleteLater()));
        m_startTime = std::chrono::high_resolution_clock::now();
        thread->start();
        reportInfo() << "FINISHED QThread setup in MovieStreamer" << reportEnd();
        m_streamIsStarted = true;
    }

    waitForFirstFrame();
    reportInfo() << "Finished movie streamer execute" << reportEnd();
}

bool MovieStreamer::hasReachedEnd() {
    return m_finished;
}

void MovieStreamer::setGrayscale(bool grayscale) {
    mGrayscale = grayscale;
}

std::string MovieStreamer::getFilename() const {
    return mFilename;
}

bool MovieStreamer::getGrayscale() const {
    return mGrayscale;
}

void MovieStreamer::setFinished(bool finished) {
    m_finished = finished;
}

MovieStreamer::~MovieStreamer() {
    reportInfo() << "Destroying movie streamer" << reportEnd();
    stop();
}

int MovieStreamer::getFramesAdded() const {
    return m_framesAdded;
}

}