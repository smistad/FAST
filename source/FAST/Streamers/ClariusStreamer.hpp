#pragma once

#include <FAST/Streamers/Streamer.hpp>
#include <thread>

struct _ClariusImageInfo;
struct _ClariusPosInfo;


namespace fast {

class FAST_EXPORT ClariusStreamer : public Streamer {
    FAST_OBJECT(ClariusStreamer)
    public:
        void setConnectionAddress(std::string ipAddress);
        void setConnectionPort(int port);
        void stop();
        ~ClariusStreamer();
        uint getNrOfFrames();
        void newImageFn(const void* newImage, const _ClariusImageInfo* nfo, int npos, const _ClariusPosInfo* pos);
		void toggleFreeze();
		void increaseDepth();
		void decreaseDepth();
	private:
        ClariusStreamer();
        void execute();
        void generateStream() override {};

        bool mStreamIsStarted;
        bool mFirstFrameIsInserted;
        bool mStop;
        bool mGrayscale;
        std::string mIPAddress = "192.168.1.1";
        int mPort = 5828;
        uint mNrOfFrames;

        std::mutex mFirstFrameMutex;
        std::condition_variable mFirstFrameCondition;
};

}