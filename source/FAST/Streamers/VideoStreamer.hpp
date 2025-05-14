#pragma once

#include "FAST/Streamers/Streamer.hpp"
#include <QObject>

class QMediaPlayer;
class QThread;
class QVideoFrame;
class QEventLoop;
class QMediaCaptureSession;
class QVideoSink;

namespace fast {

class Image;


/**
 * @brief Stream frames from video files
 *
 * This streamer uses Qt 6 to streams every frame in a movie as an Image.
 *
 * <h3>Output ports</h3>
 * - 0: Image - Movie frame
 *
 * @ingroup streamers
 * @sa VideoRandomAccessStreamer
 */
class FAST_EXPORT VideoStreamer : public Streamer {
    FAST_PROCESS_OBJECT(VideoStreamer)
    public:
        /**
         * @brief Create instance
         * @param filename Movie file to stream from
         * @param grayscale Whether to convert to grayscale or not
         * @param loop Whether to loop the video or not
         * @return instance
         */
        FAST_CONSTRUCTOR(VideoStreamer,
                         std::string, filename,,
                         bool, loop, = false,
                         bool, useFramerate, = true,
                         int, framerate, = -1,
                         bool, grayscale, = true
        );
        void setFilename(std::string filename);
        std::string getFilename() const;
        bool hasReachedEnd();
        void addNewImageFrame(const uchar* data, int width, int height, int framerate);
        void setGrayscale(bool grayscale);
        bool getGrayscale() const;
        void setFinished(bool finished);
        void setLoop(bool loop);
        bool getLoop() const;
        int getFramesAdded() const;
        void stop() override;
        ~VideoStreamer() override;
        void loadAttributes() override;
        void addLastFrame();
        bool getUseFramerate() const;
        int getRequestedFramerate() const;
    private:
        VideoStreamer();
        void execute();
        void generateStream() override;

        std::string mFilename;
        bool mGrayscale = true;
        bool m_finished = false;
        bool m_loop = false;
        bool m_useFramerate = true;
        int m_framerate = -1;
        uint64_t m_framesAdded = 0;
        std::chrono::high_resolution_clock::time_point m_startTime;
        //QThread* thread;
        QEventLoop* m_eventLoop;
        std::shared_ptr<Image> m_currentImage;

};

class VideoStreamerWorker : public QObject {
    Q_OBJECT
    public:
        VideoStreamerWorker(VideoStreamer* streamer);
        ~VideoStreamerWorker();
    public Q_SLOTS:
        void run();
    Q_SIGNALS:
        void newFrame(const QVideoFrame& frame, int framerate);
        void finished();
        void error(QString err);
    private:
        VideoStreamer* mStreamer;
        QMediaPlayer* m_player;
        QMediaCaptureSession* m_captureSession;
        QVideoSink* m_mySink;
        int m_FPS = -1;
};

}

