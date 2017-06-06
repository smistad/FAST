#ifndef DUMMY_IGTL_IMAGE_SERVER_HPP
#define DUMMY_IGTL_IMAGE_SERVER_HPP

#include "FASTExport.hpp"
#include "FAST/Streamers/ImageFileStreamer.hpp"
#include <thread>

namespace fast {

class FAST_EXPORT DummyIGTLServer {
    public:
        DummyIGTLServer();
        ~DummyIGTLServer();
        void setPort(uint port);
        void setFramesPerSecond(uint fps);
        void setMaximumFramesToSend(uint frames);
        void setImageStreamer(ImageFileStreamer::pointer streamer);
        void start();
        void stream();
    private:
        uint mPort;
        uint mFPS;
        int mFrames;
        ImageFileStreamer::pointer mStreamer;
        std::thread mThread;


};

};

#endif
