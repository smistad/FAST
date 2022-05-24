#pragma once

#include <FAST/ProcessObject.hpp>
#include <FAST/Streamers/Streamer.hpp>
#include <thread>

namespace fast {

class Image;

/**
 * @brief Converts a stream of images into stream of Batch data objects
 *
 * This is used for doing batch processing on a stream of images.
 *
 * @ingroup neural-network
 */
class FAST_EXPORT ImageToBatchGenerator : public Streamer {
    FAST_PROCESS_OBJECT(ImageToBatchGenerator)
    public:
        /**
         * @brief Create instance
         * @param maxBatchSize Maximum batch size
         * @return instance
         */
        FAST_CONSTRUCTOR(ImageToBatchGenerator, int, maxBatchSize,);
        void setMaxBatchSize(int size);
        ~ImageToBatchGenerator() override;
        void loadAttributes() override;
    protected:
        void execute() override;
        void generateStream() override;
        int m_maxBatchSize;

        DataChannel::pointer mParent;
    private:
        ImageToBatchGenerator();
};

}