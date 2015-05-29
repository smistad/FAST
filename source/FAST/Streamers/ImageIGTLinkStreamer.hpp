#ifndef IMAGE_IGTLINK_STREAMER_HPP
#define IMAGE_IGTLINK_STREAMER_HPP

#include "FAST/SmartPointers.hpp"
#include "FAST/Streamers/Streamer.hpp"
#include "FAST/ProcessObject.hpp"
#include <boost/thread.hpp>
#include "igtlClientSocket.h"

namespace fast {

class ImageIGTLinkStreamer : public Streamer, public ProcessObject {
    FAST_OBJECT(ImageIGTLinkStreamer)
    public:
        void setConnectionAddress(std::string address);
        void setConnectionPort(uint port);
        void setStreamingMode(StreamingMode mode);
        void setMaximumNumberOfFrames(uint nrOfFrames);
        bool hasReachedEnd() const;
        uint getNrOfFrames() const;
        /**
         * This method runs in a separate thread and adds frames to the
         * output object
         */
        void producerStream();

        void stop();

        ~ImageIGTLinkStreamer();
    private:
        ImageIGTLinkStreamer();

        // Update the streamer if any parameters have changed
        void execute();

        uint mNrOfFrames;
        uint mMaximumNrOfFrames;
        bool mMaximumNrOfFramesSet;

        boost::thread *thread;
        boost::mutex mFirstFrameMutex;
        boost::mutex mStopMutex;
        boost::condition_variable mFirstFrameCondition;

        bool mStreamIsStarted;
        bool mFirstFrameIsInserted;
        bool mHasReachedEnd;
        bool mStop;

        std::string mAddress;
        uint mPort;

        igtl::ClientSocket::Pointer mSocket;

};

} // end namespace

#endif
