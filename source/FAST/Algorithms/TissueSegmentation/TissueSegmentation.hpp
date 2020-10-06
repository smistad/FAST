#pragma once

#include <FAST/Algorithms/SegmentationAlgorithm.hpp>

namespace fast {

class FAST_EXPORT TissueSegmentation : public SegmentationAlgorithm {
    FAST_OBJECT(TissueSegmentation)
    public:
        /**
         * Set single threshold for pixel filtering based on the distance a RGB-triplet is from the color white. Default is 85
         * @param thresh
         */
        void setThreshold(int thresh);
        /**
         * Get current threshold value
         */
        int getThreshold() const;
        /**
         * Set radius for disk used in morphological dilation. Default is 9
         * @param radius
         */
        void setDilate(int radius);
        /**
         * Get current dilation value
         */
        int getDilate() const;
        /**
         * Set radius for disk used in morphological erosion. Default is 9
         * @param radius
         */
        void setErode(int radius);
        /**
         * Get current erosion value
         */
        int getErode() const;
    protected:
        void execute();
        TissueSegmentation();
    private:
        int m_dilate = 9;
        int m_erode = 9;
        int m_thresh = 85;
};

}