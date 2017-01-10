#ifndef DUMMY_IGTL_IMAGE_SERVER_HPP
#define DUMMY_IGTL_IMAGE_SERVER_HPP

#include "FAST/Streamers/ImageFileStreamer.hpp"
#include <thread>

namespace fast {

class DummyIGTLServer {
    public:
        DummyIGTLServer();
        ~DummyIGTLServer();
        void setPort(uint port);
        void setFramesPerSecond(uint fps);
        void setImageStreamer(ImageFileStreamer::pointer streamer);
        void start();
        void stream();
    private:
        uint mPort;
        uint mFPS;
        ImageFileStreamer::pointer mStreamer;
        std::thread mThread;

};

};

#endif
