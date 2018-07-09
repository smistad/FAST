#ifndef IGTLINK_STREAMER_HPP
#define IGTLINK_STREAMER_HPP

//#include <boost/signals2.hpp>
#include <thread>
#include <unordered_map>

#include "FAST/Streamers/Streamer.hpp"
#include "FAST/ProcessObject.hpp"
#include <igtl/igtlClientSocket.h>
#include <set>
#include "FASTExport.hpp"

namespace fast {

class Image;

class FAST_EXPORT IGTLinkStreamer : public Streamer {
    FAST_OBJECT(IGTLinkStreamer)
    public:
		std::set<std::string> getImageStreamNames();
		std::set<std::string> getTransformStreamNames();
		std::vector<std::string> getActiveImageStreamNames();
		std::vector<std::string> getActiveTransformStreamNames();
		std::string getStreamDescription(std::string streamName);
        void setConnectionAddress(std::string address);
        void setConnectionPort(uint port);
        bool hasReachedEnd();
        uint getNrOfFrames() const;

		/**
		 * Will select first image stream
		 * @return
		 */
		DataPort::pointer getOutputPort();

        template<class T>
        DataPort::pointer getOutputPort(std::string deviceName);

        /**
         * This method runs in a separate thread and adds frames to the
         * output object
         */
        void producerStream();

        void stop();

        ~IGTLinkStreamer();

        // Signals
        //boost::signals2::signal<void ()> connectionEstablishedSignal;
        //boost::signals2::signal<void ()> connectionLostSignal;
        // Ultrasound systems can freeze and thereby stop sending data, these signals are used for that
        //boost::signals2::signal<void ()> freezeSignal;
        //boost::signals2::signal<void ()> unfreezeSignal;
    private:
        IGTLinkStreamer();

        // Update the streamer if any parameters have changed
        void execute();

        uint mNrOfFrames;
        uint mMaximumNrOfFrames;
        bool mMaximumNrOfFramesSet;

        std::thread *thread;
        std::mutex mFirstFrameMutex;
        std::mutex mStopMutex;
        std::condition_variable mFirstFrameCondition;

        bool mStreamIsStarted;
        bool mFirstFrameIsInserted;
        bool mHasReachedEnd;
        bool mStop;
        bool mInFreezeMode;

        std::string mAddress;
        uint mPort;

        igtl::ClientSocket::Pointer mSocket;

		std::set<std::string> mImageStreamNames;
		std::set<std::string> mTransformStreamNames;
		std::unordered_map<std::string, std::string> mStreamDescriptions;
        std::unordered_map<std::string, uint> mOutputPortDeviceNames;

        void updateFirstFrameSetFlag();
};



template<class T>
DataPort::pointer IGTLinkStreamer::getOutputPort(std::string deviceName) {
	uint portID;
	if(mOutputPortDeviceNames.count(deviceName) == 0) {
		portID = getNrOfOutputPorts();
		createOutputPort<T>(portID);
		mOutputPortDeviceNames[deviceName] = portID;
	} else {
		portID = mOutputPortDeviceNames[deviceName];
	}
    return ProcessObject::getOutputPort(portID);
}



} // end namespace

#endif
