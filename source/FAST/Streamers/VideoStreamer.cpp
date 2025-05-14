#include "VideoStreamer.hpp"
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QApplication>
#include <QThread>
#include <QDir>
#include <FAST/Data/Image.hpp>
#include <QVideoSink>
#include <QVideoFrame>
#include <QTimer>
#include <QMediaMetaData>

namespace fast {

void VideoStreamer::addLastFrame() {
    if(m_currentImage) {
        try {
            m_currentImage->setLastFrame("VideoStreamer");
            addOutputData(0, m_currentImage);
            ++m_framesAdded;
            Reporter::info() << "Added last frame with a total of: " << m_framesAdded << Reporter::end();
            // Make sure we end the waiting thread if first frame has not been inserted
            frameAdded();
            m_currentImage.reset();
        } catch(ThreadStopped &e) {

        }
    }
}

void VideoStreamer::addNewImageFrame(const uchar* data, int width, int height, int framerate) {
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
        m_currentImage->setFrameData("video-file-framerate", std::to_string(framerate));
        addOutputData(0, m_currentImage);
        ++m_framesAdded;
        Reporter::info() << "Added frames: " << m_framesAdded << Reporter::end();
        // Make sure we end the waiting thread if first frame has not been inserted
        frameAdded();
    }
    m_currentImage = output;
}

void VideoStreamer::setFilename(std::string filename) {
    mFilename = filename;
    mIsModified = true;
}

void VideoStreamer::loadAttributes() {
    setFilename(getStringAttribute("filename"));
    setGrayscale(getBooleanAttribute("grayscale"));
}

VideoStreamer::VideoStreamer() {
    createOutputPort<Image>(0);

    createStringAttribute("filename", "Filename", "Filepath to movie file", "");
    createBooleanAttribute("grayscale", "Grayscale", "Convert movie to grayscale while streaming", mGrayscale);
    createBooleanAttribute("loop", "Loop", "Loop movie", m_loop);
}

VideoStreamer::VideoStreamer(std::string filename, bool loop, bool useFramerate, int framerate, bool grayscale) {
    createOutputPort(0, "Image");
    setFilename(filename);
    setLoop(loop);
    setGrayscale(grayscale);
    m_useFramerate = useFramerate;
    m_framerate = framerate;
}

VideoStreamerWorker::VideoStreamerWorker(VideoStreamer* streamer) {
    mStreamer = streamer;
}

VideoStreamerWorker::~VideoStreamerWorker() {
    m_player->stop();
    delete m_player; // TODO needed?
    delete m_mySink;
    Reporter::info() << "Movie streamer worker destroyed" << Reporter::end();
}

void VideoStreamerWorker::run() {
    m_player = new QMediaPlayer();
    QObject::connect(this, &VideoStreamerWorker::finished, m_player, &QMediaPlayer::deleteLater);
    m_mySink = new QVideoSink();
    connect(m_mySink, &QVideoSink::videoFrameChanged, [this](const QVideoFrame& frame) {
        emit newFrame(frame, m_FPS);
    });
    m_player->setVideoSink(m_mySink);
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
    m_player->setSource(QUrl(("file://" + prefix + mStreamer->getFilename()).c_str()));
    if(mStreamer->getLoop())
        m_player->setLoops(QMediaPlayer::Infinite);
    // Playback rate of 1.0 means original speed.
    // TODO not loaded yet?
    QObject::connect(m_player, &QMediaPlayer::mediaStatusChanged, [=](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::LoadedMedia) {
            auto metadata = m_player->metaData();
            float FPS = metadata.value(QMediaMetaData::VideoFrameRate).toFloat();
            int64_t duration = metadata.value(QMediaMetaData::Duration).toInt();
            //std::cout << "FPS is: " << FPS << std::endl;
            m_FPS = round(FPS);
            //std::cout << "Duration is: " << duration << " ms" << std::endl;
            //std::cout << "Approx nr of frames: " << (float)duration/FPS << " frames" << std::endl;
            if(mStreamer->getUseFramerate()) {
                m_player->setPlaybackRate(1.0); // Number of frames outputted seems to be constant despite playback speed
            } else {
                int requestedFramerate = mStreamer->getRequestedFramerate();
                if(requestedFramerate < 0) {
                    m_player->setPlaybackRate(1000);
                } else {
                    m_player->setPlaybackRate(requestedFramerate/FPS);
                }
            }
            m_player->play(); // This does not block
            Reporter::info() << "Play returned" << Reporter::end();
        } else if(status == QMediaPlayer::EndOfMedia) {
            emit finished();
        }
    });
}


void VideoStreamer::execute() {
    if(!m_streamIsStarted) {
        if(!fileExists(mFilename))
            throw FileNotFoundException(mFilename);
    }
    startStream();

    waitForFirstFrame();
    reportInfo() << "Finished movie streamer execute" << reportEnd();
}

bool VideoStreamer::hasReachedEnd() {
    return m_finished;
}

void VideoStreamer::setGrayscale(bool grayscale) {
    mGrayscale = grayscale;
}

std::string VideoStreamer::getFilename() const {
    return mFilename;
}

bool VideoStreamer::getGrayscale() const {
    return mGrayscale;
}

void VideoStreamer::setFinished(bool finished) {
    m_finished = finished;
}

VideoStreamer::~VideoStreamer() {
    reportInfo() << "Destroying movie streamer" << reportEnd();
    stop();
    reportInfo() << "Done destroying video streamer" << reportEnd();
}

int VideoStreamer::getFramesAdded() const {
    return m_framesAdded;
}

void VideoStreamer::setLoop(bool loop) {
    m_loop = loop;
    setModified(true);
}

bool VideoStreamer::getLoop() const {
    return m_loop;
}

void VideoStreamer::generateStream() {
    //thread = new QThread;
    auto worker = new VideoStreamerWorker(this);
    //worker->moveToThread(thread);
    //QObject::connect(worker, SIGNAL(error(QString)), this, SLOT(errorString(QString)));
    /*
    QObject::connect(thread, SIGNAL(started()), worker, SLOT(run()));
    QObject::connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
    QObject::connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
    QObject::connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    QObject::connect(thread, SIGNAL(finished()), worker, SLOT(deleteLater()));
     */
    QObject::connect(worker, &VideoStreamerWorker::finished, [this]() {
        if(m_eventLoop != nullptr)
            m_eventLoop->quit();
    });
    QObject::connect(worker, &VideoStreamerWorker::newFrame, [this](const QVideoFrame& frame, int framerate) {
        if(frame.isValid()) {
            // TODO Check if this actually runs in worker thread
            const int width = frame.width();
            const int height = frame.height();
            QImage image = frame.toImage();

            QImage::Format format;
            if(getGrayscale()) {
                format = QImage::Format_Grayscale8;
            } else {
                format = QImage::Format_RGB888;
            }
            QImage convertedImage = image.convertToFormat(format);
            try {
                addNewImageFrame(convertedImage.constBits(), width, height, framerate);
                Reporter::info() << "Finished processing movie frame" << Reporter::end();
            } catch(ThreadStopped & e) {
            }
        } else {
            //std::cout << "INVALID FRAME " << std::endl;
        }
    });
    m_startTime = std::chrono::high_resolution_clock::now();
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
    Reporter::info() << "Exited event loop of VideoStreamer" << Reporter::end();
    addLastFrame();
    delete worker;
    delete m_eventLoop;
    delete timer;
    m_eventLoop = nullptr;
    m_finished = true;
}

void VideoStreamer::stop() {
    reportInfo() << "Video streamer stopped called." << reportEnd();
    //thread->quit();
    if(m_eventLoop != nullptr)
        m_eventLoop->quit();
    Streamer::stop();
}

bool VideoStreamer::getUseFramerate() const {
    return m_useFramerate;
}

int VideoStreamer::getRequestedFramerate() const {
    return m_framerate;
}

}