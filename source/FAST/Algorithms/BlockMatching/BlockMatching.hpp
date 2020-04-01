#pragma once

#include <FAST/ProcessObject.hpp>
#include <deque>

namespace fast {

class Image;

/**
 * 2D block matching on the GPU. Input is a stream of input images, output is a stream of images
 * with 2 channels giving the x,y motion of each pixel.
 */
class FAST_EXPORT BlockMatching : public ProcessObject {
    FAST_OBJECT(BlockMatching)
    public:
        enum class MatchingMetric {
            NORMALIZED_CROSS_CORRELATION,
            SUM_OF_SQUARED_DIFFERENCES,
            SUM_OF_ABSOLUTE_DIFFERENCES,
        };

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
        BlockMatching();
        void execute() override;

        MatchingMetric m_type = MatchingMetric::SUM_OF_ABSOLUTE_DIFFERENCES;
        int m_blockSizeHalf = 5;
        int m_searchSizeHalf = 5;
        float m_intensityThreshold = std::numeric_limits<float>::min();
        int m_timeLag = 1;
        bool m_forwardBackward = false;
        Vector2i m_offsetROI = Vector2i::Zero();
        Vector2i m_sizeROI = Vector2i::Zero();
        std::deque<SharedPointer<Image>> m_frameBuffer;

};

}