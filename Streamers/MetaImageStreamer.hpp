#ifndef META_IMAGE_STREAMER_HPP
#define META_IMAGE_STREAMER_HPP

#include "SmartPointers.hpp"
#include "Streamer.hpp"
#include "DynamicImage.hpp"
#include <boost/thread.hpp>

namespace fast {

class MetaImageStreamer : public Streamer {
    FAST_OBJECT(MetaImageStreamer)
    public:
        DynamicImage::pointer getOutput();
        void setFilenameFormat(std::string str);
        void setDevice(ExecutionDevice::pointer device);
        bool hasReachedEnd() const;
        // This method runs in a separate thread and adds frames to the
        // output object
        void producerStream();

        ~MetaImageStreamer();
    private:
        MetaImageStreamer();

        // A reference to the output object used to update its next frame
        DynamicImage::pointer mOutput;

        // Update the streamer if any parameters have changed
        void execute();


        boost::thread *thread;

        bool mStreamIsStarted;
        bool mFirstFrameIsInserted;
        bool mHasReachedEnd;

        std::string mFilenameFormat;

        ExecutionDevice::pointer mDevice;

};

} // end namespace fast

#endif
