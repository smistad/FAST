#ifndef TRANSFORM_IGTLINK_STREAMER_HPP
#define TRANSFORM_IGTLINK_STREAMER_HPP

#include "SmartPointers.hpp"
#include "Streamer.hpp"
#include "ProcessObject.hpp"
#include <boost/thread.hpp>
#include "igtlClientSocket.h"

namespace fast {

class TransformIGTLinkStreamer : public Streamer, public ProcessObject {
    FAST_OBJECT(TransformIGTLinkStreamer)
    public:
        void setTransformName(std::string transformName);
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

        ~TransformIGTLinkStreamer();
    private:
        TransformIGTLinkStreamer();

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

        std::string mTransformName;
        std::string mAddress;
        uint mPort;

        igtl::ClientSocket::Pointer mSocket;

};

} // end namespace

#endif
