#pragma once

#include <thread>
#include <unordered_map>
#include "FAST/Streamers/Streamer.hpp"
#include "FAST/ProcessObject.hpp"
#include "FAST/Data/SimpleDataObject.hpp"
#include <set>
#include "FASTExport.hpp"
#include <deque>
#include <string>

// Forward declare

namespace fast {

class Image;
class IGTLSocketWrapper;

// Should be moved somewhere else, but for now it is only used by OpenIGTLinkStreamer
FAST_SIMPLE_DATA_OBJECT(String, std::string);


/**
 * @brief Stream image or transforms from an OpenIGTLink server
 *
 * This streamer uses the OpenIGTLink protocol and library to stream data such as images and transforms from a server
 *
 * Default streaming mode is StreamingMode::NewestFrameOnly
 *
 * <h3>Output ports</h3>
 * Multiple ports possible dependeing on number of streams from OpenIGTLink server
 *
 * @ingroup streamers
 */
class FAST_EXPORT OpenIGTLinkStreamer : public Streamer {
    FAST_PROCESS_OBJECT(OpenIGTLinkStreamer)
    public:
        /**
         * @brief Create instance
         * @param ipAddress IP address of server to connect to. Default is localhost
         * @param port Port of server to connect to. Default is 18944
         * @return instance
         */
        FAST_CONSTRUCTOR(OpenIGTLinkStreamer,
                         std::string, ipAddress, = "localhost",
                         int, port, = 18944
        );
		std::set<std::string> getImageStreamNames();
		std::set<std::string> getTransformStreamNames();
		std::vector<std::string> getActiveImageStreamNames();
		std::vector<std::string> getActiveTransformStreamNames();
		std::string getStreamDescription(std::string streamName);
        void setConnectionAddress(std::string address);
        void setConnectionPort(uint port);
        uint getNrOfFrames() const;
        /**
         * Get output port number for specific device
         * @param deviceName
         * @return output port number
         */
        uint getOutputPortNumber(std::string deviceName);

		/**
		 * Will select first image stream
		 * @return
		 */
		DataChannel::pointer getOutputPort(uint portID = 0) override;

        DataChannel::pointer getOutputPort(std::string deviceName);

        // V4 TODO: Need a method to set which output port related to device name
        uint createOutputPortForDevice(std::string deviceName);

        /**
         * This method runs in a separate thread and adds frames to the
         * output object
         */
        void generateStream() override;

        ~OpenIGTLinkStreamer();
        void loadAttributes() override;

        float getCurrentFramerate();
    private:
        // Update the streamer if any parameters have changed
        void execute();

        void addTimestamp(uint64_t timestamp);

        uint mNrOfFrames;
        uint mMaximumNrOfFrames;
        bool mMaximumNrOfFramesSet;
        std::deque<uint64_t> m_timestamps;

        bool mInFreezeMode;

        std::string mAddress;
        uint mPort;

		IGTLSocketWrapper* mSocketWrapper;
        //igtl::ClientSocket::Pointer mSocket;

		std::set<std::string> mImageStreamNames;
		std::set<std::string> mTransformStreamNames;
		std::unordered_map<std::string, std::string> mStreamDescriptions;
        std::unordered_map<std::string, uint> mOutputPortDeviceNames;

        void updateFirstFrameSetFlag();
};








} // end namespace

