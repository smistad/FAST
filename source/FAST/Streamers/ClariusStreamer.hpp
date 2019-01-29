#pragma once

#include <FAST/Streamers/Streamer.hpp>
#include <thread>

struct _ClariusImageInfo;
struct _ClariusPosInfo;


namespace fast {

class ClariusStreamer : public Streamer {
    FAST_OBJECT(ClariusStreamer)
    public:
        void producerStream();
        bool hasReachedEnd();
        void stop();
        ~ClariusStreamer();
        uint getNrOfFrames();
        void newImageFn(const void* newImage, const _ClariusImageInfo* nfo, int npos, const _ClariusPosInfo* pos);
    private:
        ClariusStreamer();
        void execute();

        bool mStreamIsStarted;
        bool mFirstFrameIsInserted;
        bool mHasReachedEnd;
        bool mStop;
        uint mNrOfFrames;

        std::unique_ptr<std::thread> mThread;
        std::mutex mFirstFrameMutex;
        std::mutex mStopMutex;
        std::condition_variable mFirstFrameCondition;
};

}