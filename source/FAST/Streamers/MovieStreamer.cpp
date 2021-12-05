#include "MovieStreamer.hpp"
#include <QtMultimedia/QAbstractVideoSurface>
#include <QtMultimedia/QMediaPlayer>
#include <QtMultimediaWidgets/QVideoWidget>
#include <QApplication>
#include <QThread>
#include <QDir>
#include <QMediaPlaylist>
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
            streamer->addLastFrame();
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
            streamer->addLastFrame();
            Reporter::info() << "QMediaPlayer state changed to stopped - stopping stream" << Reporter::end();
            streamer->setFinished(true);
        }
    }

    ~VideoSurface() {
        Reporter::info() << "Video surface of movie streamer was destroyed" << Reporter::end();
    }
};

void MovieStreamer::addLastFrame() {
    if(m_currentImage) {
        m_currentImage->setLastFrame("MovieStreamer");
		addOutputData(0, m_currentImage);
        ++m_framesAdded;
        Reporter::info() << "Added last frame with a total of: " << m_framesAdded << Reporter::end();
        // Make sure we end the waiting thread if first frame has not been inserted
        frameAdded();
    }
}

void MovieStreamer::addNewImageFrame(const uchar* data, int width, int height) {
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

    if(m_currentImage) {
        addOutputData(0, m_currentImage);
        ++m_framesAdded;
        Reporter::info() << "Added frames: " << m_framesAdded << Reporter::end();
        // Make sure we end the waiting thread if first frame has not been inserted
        frameAdded();
    }
    m_currentImage = output;
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
    createBooleanAttribute("loop", "Loop", "Loop movie", m_loop);
}

MovieStreamer::MovieStreamer(std::string filename, bool grayscale, bool loop) {
    createOutputPort(0, "Image");
    setFilename(filename);
    setLoop(loop);
    setGrayscale(grayscale);
}

MovieStreamerWorker::MovieStreamerWorker(MovieStreamer* streamer) {
    mStreamer = streamer;
}

MovieStreamerWorker::~MovieStreamerWorker() {
    delete m_myVideoSurface;
    Reporter::info() << "Movie streamer worker destroyed" << Reporter::end();
}

void MovieStreamerWorker::run() {
    m_player = new QMediaPlayer();
    QObject::connect(this, &MovieStreamerWorker::finished, m_player, &QMediaPlayer::deleteLater);
    //std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    m_myVideoSurface = new VideoSurface();
    m_myVideoSurface->streamer = mStreamer;
    m_player->setMuted(true);
    m_player->setVideoOutput(m_myVideoSurface);
    QObject::connect(m_player, &QMediaPlayer::stateChanged, std::bind(&VideoSurface::stateChanged, m_myVideoSurface, std::placeholders::_1));
    std::string prefix = "";
    if(mStreamer->getFilename()[0] != '/' && mStreamer->getFilename()[1] == ':') { // On windows w want file:///C:/asd/asd..
        prefix = "/";
    } else if (mStreamer->getFilename()[0] != '/' && mStreamer->getFilename()[1] != ':') {
        // Relative path, make into absolute path:
#ifdef WIN32
        prefix = "/" + QDir::currentPath().toStdString() + "/";
#else
        prefix = QDir::currentPath().toStdString() + "/";
#endif
    }
    auto playlist = new QMediaPlaylist();
    playlist->addMedia(QUrl(("file://" + prefix + mStreamer->getFilename()).c_str()));
    if(mStreamer->getLoop()) {
        playlist->setPlaybackMode(QMediaPlaylist::Loop);
    }
    m_player->setPlaylist(playlist);
    m_player->play(); // This does not block
    Reporter::info() << "Play returned" << Reporter::end();
}


void MovieStreamer::execute() {
    if(!m_streamIsStarted) {
        if(!fileExists(mFilename))
            throw FileNotFoundException(mFilename);
        thread = new QThread;
        MovieStreamerWorker* worker = new MovieStreamerWorker(this);
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
    if(thread != nullptr)
		thread->quit();
    stop();
}

int MovieStreamer::getFramesAdded() const {
    return m_framesAdded;
}

void MovieStreamer::setLoop(bool loop) {
    m_loop = loop;
    setModified(true);
}

bool MovieStreamer::getLoop() const {
    return m_loop;
}

}