#pragma once

#include <FAST/ProcessObject.hpp>
#include <deque>

namespace fast {

class Image;

/**
 * @brief Matching metrics for tracking
 * @ingroup motion-and-tracking
 */
enum class MatchingMetric {
    NORMALIZED_CROSS_CORRELATION,
    SUM_OF_SQUARED_DIFFERENCES,
    SUM_OF_ABSOLUTE_DIFFERENCES,
};

/**
 * @brief Block matching tracking of an image stream.
 *
 * 2D block matching on the GPU. Input is a stream of input images, output is a stream of images
 * with 2 channels giving the x,y motion of each pixel.
 *
 * @ingroup motion-and-tracking
 */
class FAST_EXPORT BlockMatching : public ProcessObject {
    FAST_PROCESS_OBJECT(BlockMatching)
    public:
        /**
         * @brief Create instance
         * @param blockSize Must be odd.
         * @param searchSize Must be odd.
         * @param metric Similarity metric to use
         * @param forwardBackwardTracking Do forward-backward tracking and take average of the two. Will be slover.
         * @param timeLag How many frames to skip when comparing. Default is 1, comarping frame t vs frame t-1.
         * @return instance
         */
        FAST_CONSTRUCTOR(BlockMatching,
             int, blockSize, = 11,
             int, searchSize, = 11,
             MatchingMetric, metric, = MatchingMetric::SUM_OF_ABSOLUTE_DIFFERENCES,
             bool, forwardBackwardTracking, = false,
             int, timeLag, = 1
         );

        /**
         * Convert string of metric to type
         * @param name
         * @return MatchingMetric
         */
        static MatchingMetric stringToMetric(std::string name) {
            std::map<std::string, MatchingMetric> map = {
                {"NCC", MatchingMetric::NORMALIZED_CROSS_CORRELATION},
                {"SAD", MatchingMetric::SUM_OF_ABSOLUTE_DIFFERENCES},
                {"SSD", MatchingMetric::SUM_OF_SQUARED_DIFFERENCES},
            };
            return map.at(name);
        }

        /**
         * Select which matching metric to use
         * @param type
         */
        void setMatchingMetric(MatchingMetric type);
        /**
         * Set size of the blocks to match. Has to be odd
         * @param size
         */
        void setBlockSize(int size);
        /**
         * Set size of search grid around x,y. Has to be odd
         * @param size
         */
        void setSearchSize(int size);
        /**
         * Set an intensity threshold, do not do block matching on pixels where the mean of the block is below this threhsold.
         * @param value
         */
        void setIntensityThreshold(float value);
        /**
         * Set time lag of block matching. A time lag of 2 will use frame t, and t-2 for block matching. Default is 1
         * @param timeLag
         */
        void setTimeLag(int timeLag);
        /**
         * Set whether to use forward-backward tracking or not. Disabled by default.
         * @param forwardBackward
         */
        void setForwardBackwardTracking(bool forwardBackward);
        /**
         * Set a region of interest (ROI) to run the block matching in.
         * @param offset from origin to start the ROI in pixels
         * @param size of the ROI in pixels
         */
        void setRegionOfInterest(Vector2i offset, Vector2i size);
        void loadAttributes() override;
    private:
        void execute() override;

        MatchingMetric m_type = MatchingMetric::SUM_OF_ABSOLUTE_DIFFERENCES;
        int m_blockSizeHalf = 5;
        int m_searchSizeHalf = 5;
        float m_intensityThreshold = std::numeric_limits<float>::min();
        int m_timeLag = 1;
        bool m_forwardBackward = false;
        Vector2i m_offsetROI = Vector2i::Zero();
        Vector2i m_sizeROI = Vector2i::Zero();
        std::deque<std::shared_ptr<Image>> m_frameBuffer;

};

}