#ifndef IGTLINK_STREAMER_HPP
#define IGTLINK_STREAMER_HPP

#include "FAST/SmartPointers.hpp"
#include "FAST/Streamers/Streamer.hpp"
#include "FAST/ProcessObject.hpp"
#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>
#include "igtlClientSocket.h"

namespace fast {

class IGTLinkStreamer : public Streamer, public ProcessObject {
    FAST_OBJECT(IGTLinkStreamer)
    public:
        void setConnectionAddress(std::string address);
        void setConnectionPort(uint port);
        void setStreamingMode(StreamingMode mode);
        void setMaximumNumberOfFrames(uint nrOfFrames);
        bool hasReachedEnd() const;
        uint getNrOfFrames() const;

        template<class T>
        ProcessObjectPort getOutputPort(std::string deviceName);

        /**
         * This method runs in a separate thread and adds frames to the
         * output object
         */
        void producerStream();

        void stop();

        ~IGTLinkStreamer();
    private:
        IGTLinkStreamer();

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

        boost::unordered_map<std::string, uint> mOutputPortDeviceNames;

        template <class T>
        DynamicData::pointer getOutputDataFromDeviceName(std::string deviceName);
        void updateFirstFrameSetFlag();

};


template<class T>
ProcessObjectPort IGTLinkStreamer::getOutputPort(std::string deviceName) {
    uint portID = getNrOfOutputPorts();
    createOutputPort<T>(portID, OUTPUT_DYNAMIC);
    getOutputData<T>(portID); // This initializes the output data
    mOutputPortDeviceNames[deviceName] = portID;
    return ProcessObject::getOutputPort(portID);
}

template<class T>
DynamicData::pointer IGTLinkStreamer::getOutputDataFromDeviceName(std::string deviceName) {
    if(mOutputPortDeviceNames.count(deviceName) == 0)
        throw Exception("Output port associated with device name " + deviceName + " not found in the IGTLinkStreamer.");

    return getOutputData<T>(mOutputPortDeviceNames[deviceName]);
}

} // end namespace

#endif
