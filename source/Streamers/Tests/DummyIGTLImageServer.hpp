#ifndef DUMMY_IGTL_IMAGE_SERVER_HPP
#define DUMMY_IGTL_IMAGE_SERVER_HPP

#include "ImageFileStreamer.hpp"
#include <boost/thread.hpp>

namespace fast {

class DummyIGTLImageServer {
    public:
        DummyIGTLImageServer();
        ~DummyIGTLImageServer();
        void setPort(uint port);
        void setFramesPerSecond(uint fps);
        void setImageStreamer(ImageFileStreamer::pointer streamer);
        void start();
        void streamImages();
    private:
        uint mPort;
        uint mFPS;
        ImageFileStreamer::pointer mStreamer;
        boost::thread mThread;

};

};

#endif
