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
        std::cout << "Movie frame received!" << std::endl;
        std::cout << frame.pixelFormat() << std::endl;

        QVideoFrame cloneFrame(frame);
        cloneFrame.map(QAbstractVideoBuffer::ReadOnly);
        const int width = frame.width();
        const int height = frame.height();
        QImage image(cloneFrame.bits(), width, height, frame.bytesPerLine(), QVideoFrame::imageFormatFromPixelFormat(frame.pixelFormat()));
        std::cout << "Finished copying QVideoFrame to QImage" << std::endl;

        QImage::Format format;
        if(streamer->getGrayscale()) {
            format = QImage::Format_Grayscale8;
        } else {
            format = QImage::Format_RGB888;
        }
        QImage convertedImage = image.convertToFormat(format);
        std::cout << "Finished converting QImage to uint8 format" << std::endl;
        streamer->addNewImageFrame(convertedImage.constBits(), width, height);

        std::cout << "Finished present" << std::endl;

        // Handle the frame and do your processing
        return true;
    }
};

void MovieStreamer::addNewImageFrame(const uchar* data, int width, int height) {
    Image::pointer output = getOutputData<Image>();
    output->create(
            width,
            height,
            TYPE_UINT8,
            mGrayscale ? 1 : 3,
            getMainDevice(),
            data
    );
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
    QMediaPlayer* player = new QMediaPlayer;
    std::cout << "Play.." << std::endl;
    //std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    VideoSurface* myVideoSurface = new VideoSurface;
    myVideoSurface->streamer = mStreamer;
    player->setVideoOutput(myVideoSurface);
    player->setMedia(QUrl(("file://" + mStreamer->getFilename()).c_str()));
    player->play();
    std::cout << "Play returned" << std::endl;
    //emit finished();
}


void MovieStreamer::execute() {
    if(!mStreamIsStarted) {
        if(!fileExists(mFilename))
            throw FileNotFoundException(mFilename);
        QThread* thread = new QThread;
        Worker* worker = new Worker(this);
        worker->moveToThread(thread);
        //QObject::connect(worker, SIGNAL(error(QString)), this, SLOT(errorString(QString)));
        QObject::connect(thread, SIGNAL(started()), worker, SLOT(run()));
        QObject::connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
        QObject::connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
        QObject::connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
        thread->start();
        reportInfo() << "FINISHED QThread setup in MovieStreamer" << reportEnd();
        mStreamIsStarted = true;
    }

    // Wait here for first frame
    std::unique_lock<std::mutex> lock(mFirstFrameMutex);
    while(!mFirstFrameIsInserted) {
        mFirstFrameCondition.wait(lock);
    }
    std::cout << "Finished movie streamer execute" << std::endl;
}

bool MovieStreamer::hasReachedEnd() {
    return false;
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

}