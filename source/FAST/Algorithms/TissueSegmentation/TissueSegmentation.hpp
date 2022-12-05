#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

/**
 * @brief Segment tissue from a WSI
 *
 * Uses the GPU to perform a simple threshold-based tissue/glass segmentation of a WSI.
 * Since glass is almost white, the thresold is the distance from the pixels color
 * to white (255,255,255).
 * A morphological closing (dilation+erosion) is performed after thresholding.
 *
 * Inputs:
 * - 0: ImagePyramid WSI
 *
 * Outputs:
 * - 0: Segmentation tissue mask
 *
 * @ingroup segmentation wsi
 */
class FAST_EXPORT TissueSegmentation : public ProcessObject {
    FAST_PROCESS_OBJECT(TissueSegmentation)
    public:
        /**
         * Create an instance of TissueSegmentation
         *
         * @param threshold Threshold for distance from white (255,255,255) to be considered as tissue.
         * @param dilationSize Size of dilation to perform after thresholding
         * @param erosionSize Size of erosion to perform after thresholding
         * @return A tissue segmentation instance
         */
        FAST_CONSTRUCTOR(TissueSegmentation,
                         int, threshold, = 85,
                         int, dilationSize, = 9,
                         int, erosionSize, = 9,
                         bool, filterZeros, = true
         )
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
        /**
         * Set status for filtering zero uints.
         */
        void setFilterZeros(bool value);
        /**
         * Get current status whether to include zero uints into background class or not. Default is true
         */
        bool getFilterZeros() const;
        void loadAttributes() override;
    protected:
        void execute() override;
    private:
        int m_dilate = 9;
        int m_erode = 9;
        int m_thresh = 85;
        bool m_filterZeros = true;
};

}