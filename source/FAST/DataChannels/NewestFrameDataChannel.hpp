#pragma once

#include <FAST/DataChannels/DataChannel.hpp>
#include <queue>

namespace fast {

/**
 * This data channel is used on the output data channels
 * of streamers when streaming mode is NEWEST_FRAME_ONLY
 */
class FAST_EXPORT NewestFrameDataChannel : public DataChannel {
    FAST_OBJECT(NewestFrameDataChannel)
    public:
        /**
         * Add frame to the data channel
         */
        virtual void addFrame(DataObject::pointer data) override;

        /**
         * @return the number of frames stored in this DataChannel
         */
        int getSize() override;

        /**
         * Set the maximum nr of frames that can be stored in this data channel.
         * Not used in this data channel.
         */
        void setMaximumNumberOfFrames(uint frames) override;

        int getMaximumNumberOfFrames() const override;

        /**
         * @brief This will unblock if this DataChannel is currently blocking. Used to stop a pipeline.
         * @param Error message to supply.
         */
        void stop(std::string errorMessage) override;

        // TODO consider removing, it is equal to getSize() > 0 atm
        bool hasCurrentData() override;

        /**
         * Get current frame, throws if current frame is not available.
         */
        DataObject::pointer getFrame() override;
    protected:
        std::condition_variable m_frameConditionVariable;
        std::shared_ptr<DataObject> m_frame;

        DataObject::pointer getNextDataFrame() override;

};

}
