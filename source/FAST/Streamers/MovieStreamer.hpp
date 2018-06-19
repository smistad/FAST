#ifndef FAST_MOVIE_STREAMER_HPP_
#define FAST_MOVIE_STREAMER_HPP_

#include "FAST/Streamers/Streamer.hpp"
#include <QObject>

class QMediaPlayer;
class QThread;

namespace fast {

class Worker;

class FAST_EXPORT MovieStreamer : public Streamer {
    FAST_OBJECT(MovieStreamer)
    public:
        void setFilename(std::string filename);
        std::string getFilename() const;
        bool hasReachedEnd();
        void addNewImageFrame(const uchar* data, int width, int height);
        void setGrayscale(bool grayscale);
        bool getGrayscale() const;
        void setFinished(bool finished);
        int getFramesAdded() const;
        ~MovieStreamer();
    private:
        MovieStreamer();
        void execute();

        std::string mFilename;
        bool mGrayscale = true;
        bool mStreamIsStarted = false;
        bool mFirstFrameIsInserted = false;
        bool m_finished = false;
        int64_t m_framesAdded = 0;
        std::mutex mFirstFrameMutex;
        std::mutex mStopMutex;
        std::condition_variable mFirstFrameCondition;
        std::chrono::high_resolution_clock::time_point m_startTime;
        QThread* thread;
        Worker* worker;

};

class Worker : public QObject {
    Q_OBJECT
    public:
        Worker(MovieStreamer* streamer);
        ~Worker();
    public slots:
        void run();
    signals:
        void finished();
        void error(QString err);
    private:
        MovieStreamer* mStreamer;
        std::unique_ptr<QMediaPlayer> m_player;
};

}

#endif
