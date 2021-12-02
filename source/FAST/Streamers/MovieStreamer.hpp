#pragma once

#include "FAST/Streamers/Streamer.hpp"
#include <QObject>

class QMediaPlayer;
class QThread;

namespace fast {

class Image;

class MovieStreamerWorker;

/**
 * @brief Stream movie files
 *
 * This streamer uses Qt 5 to streams every frame in a movie as an Image.
 *
 * <h3>Output ports</h3>
 * - 0: Image - Movie frame
 *
 * @ingroup streamers
 */
class FAST_EXPORT MovieStreamer : public Streamer {
    FAST_PROCESS_OBJECT(MovieStreamer)
    public:
        /**
         * @brief Create instance
         * @param filename Movie file to stream from
         * @param grayscale Whether to convert to grayscale or not
         * @param loop Whether to loop the video or not
         * @return instance
         */
        FAST_CONSTRUCTOR(MovieStreamer,
                         std::string, filename,,
                         bool, grayscale, = true,
                         bool, loop, = false
        );
        void setFilename(std::string filename);
        std::string getFilename() const;
        bool hasReachedEnd();
        void addNewImageFrame(const uchar* data, int width, int height);
        void setGrayscale(bool grayscale);
        bool getGrayscale() const;
        void setFinished(bool finished);
        void setLoop(bool loop);
        bool getLoop() const;
        int getFramesAdded() const;
        ~MovieStreamer();
        void loadAttributes() override;
        void addLastFrame();
    private:
        MovieStreamer();
        void execute();
        void generateStream() override {};

        std::string mFilename;
        bool mGrayscale = true;
        bool m_finished = false;
        bool m_loop = false;
        int64_t m_framesAdded = 0;
        std::chrono::high_resolution_clock::time_point m_startTime;
        QThread* thread;
        MovieStreamerWorker* worker;
        std::shared_ptr<Image> m_currentImage;

};

class VideoSurface;

class MovieStreamerWorker : public QObject {
    Q_OBJECT
    public:
        MovieStreamerWorker(MovieStreamer* streamer);
        ~MovieStreamerWorker();
    public Q_SLOTS:
        void run();
    Q_SIGNALS:
        void finished();
        void error(QString err);
    private:
        MovieStreamer* mStreamer;
        QMediaPlayer* m_player;
        VideoSurface* m_myVideoSurface;
};

}

