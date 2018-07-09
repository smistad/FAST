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
        Q_UNUSED(frame);
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

        // Handle the frame and do your processing
        return true;
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
    std::cout << m_framesAdded << std::endl;
    // Make sure we end the waiting thread if first frame has not been inserted
    {
        std::lock_guard<std::mutex> lock(mFirstFrameMutex);
        if(!mFirstFrameIsInserted)
            mFirstFrameIsInserted = true;
    }
    mFirstFrameCondition.notify_one();
}

void MovieStreamer::setFilename(std::string filename) {
    mFilename = filename;
    mIsModified = true;
}

MovieStreamer::MovieStreamer() {
    createOutputPort<Image>(0);
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
    m_player->setVideoOutput(myVideoSurface);
    m_player->setMedia(QUrl(("file://" + mStreamer->getFilename()).c_str()));
    m_player->play();
    Reporter::info() << "Play returned" << Reporter::end();
    //emit finished();
}


void MovieStreamer::execute() {
    if(!mStreamIsStarted) {
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
        QObject::connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
        QObject::connect(thread, SIGNAL(finished()), worker, SLOT(deleteLater()));
        m_startTime = std::chrono::high_resolution_clock::now();
        thread->start();
        reportInfo() << "FINISHED QThread setup in MovieStreamer" << reportEnd();
        mStreamIsStarted = true;
    }

    // Wait here for first frame
    std::unique_lock<std::mutex> lock(mFirstFrameMutex);
    while(!mFirstFrameIsInserted) {
        mFirstFrameCondition.wait(lock);
    }
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
    if(mStreamIsStarted) {
        thread->quit();
    }
}

int MovieStreamer::getFramesAdded() const {
    return m_framesAdded;
}

}