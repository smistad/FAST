#pragma once

#include <FAST/DataChannels/DataChannel.hpp>
#include <queue>
#include <FAST/Semaphore.hpp>

namespace fast {

/**
 * This queued data channel implements the producser-consumer task
 * using a lightweight semaphore. It is used on the output data channelsl
 * of streamers when streaming mode is PROCESS_ALL_FRAMES
 */
class FAST_EXPORT QueuedDataChannel : public DataChannel {
    FAST_OBJECT(QueuedDataChannel)
    public:
        /**
         * Add frame to the data channel. This call may block
         * if the buffer is full.
         */
        void addFrame(DataObject::pointer data) override;

        /**
         * @return the number of frames stored in this DataChannel
         */
        int getSize() override;

        /**
         * Set the maximum nr of frames that can be stored in this data channel
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
        std::queue<std::shared_ptr<DataObject>> m_queue;
        uint mMaximumNumberOfFrames;
        std::unique_ptr<LightweightSemaphore> m_fillCount;
        std::unique_ptr<LightweightSemaphore> m_emptyCount;

        DataObject::pointer getNextDataFrame() override;
        QueuedDataChannel();

};

}
