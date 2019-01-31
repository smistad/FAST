#pragma once

#include <FAST/Streamers/Streamer.hpp>
#include <thread>

struct _ClariusImageInfo;
struct _ClariusPosInfo;


namespace fast {

class FAST_EXPORT ClariusStreamer : public Streamer {
    FAST_OBJECT(ClariusStreamer)
    public:
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
        bool mGrayscale;
        uint mNrOfFrames;

        std::mutex mFirstFrameMutex;
        std::condition_variable mFirstFrameCondition;
};

}