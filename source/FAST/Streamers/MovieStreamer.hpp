#ifndef FAST_MOVIE_STREAMER_HPP_
#define FAST_MOVIE_STREAMER_HPP_

#include "FAST/Streamers/Streamer.hpp"
#include <QObject>

namespace fast {

class FAST_EXPORT MovieStreamer : public Streamer {
    FAST_OBJECT(MovieStreamer)
    public:
        void setFilename(std::string filename);
        std::string getFilename() const;
        bool hasReachedEnd();
        void addNewImageFrame(const uchar* data, int width, int height);
        void setGrayscale(bool grayscale);
        bool getGrayscale() const;
    private:
        MovieStreamer();
        void execute();

        std::string mFilename;
        bool mGrayscale = true;
        bool mStreamIsStarted = false;
        bool mFirstFrameIsInserted = false;
        std::mutex mFirstFrameMutex;
        std::mutex mStopMutex;
        std::condition_variable mFirstFrameCondition;

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
};

}

#endif
