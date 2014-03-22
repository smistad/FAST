#ifndef IMAGE_STREAMER_2D_HPP
#define IMAGE_STREAMER_2D_HPP

#include "Streamer.hpp"
#include "Image2Dt.hpp"
#include <boost/thread.hpp>

namespace fast {

class ImageStreamer2D : public Streamer {
    FAST_OBJECT(ImageStreamer2D)
    public:
        Image2Dt::Ptr getOutput();
        void setFilenameFormat(std::string str);
        void setDevice(ExecutionDevice::Ptr device);
        // This method runs in a separate thread and adds frames to the
        // output object
        void producerStream();

        ~ImageStreamer2D();
    private:
        ImageStreamer2D();

        // A reference to the output object used to update its next frame
        Image2Dt::Ptr mOutput;

        // Update the streamer if any parameters have changed
        void execute();


        boost::thread *thread;

        bool mStreamIsStarted;

        std::string mFilenameFormat;

        ExecutionDevice::Ptr mDevice;

};

} // end namespace fast

#endif
