#pragma once

#include "FAST/ProcessObject.hpp"

namespace fast {

/**
 * @brief Automatic cropping of an ultrasound scanner image
 *
 * When streaming or storing images/video on ultrasound scanner, the menu and a lot of information around the actual
 * ultrasound image is often included.
 * This algorithm tries to extract the ultrasound image from the entire scanner GUI image using two thresholds.
 * It does this by counting the number of non-zero pixels in every row and column of the image.
 * To find the min x and max x positions, it starts from the middle and stops at the first x position which has a
 * number of pixels equal to 0 above the thresholdVertical. It does this both in the left and right direction.
 * Then this is repeated for the vertical direction, y using thresholdHorizontal.
 * Works best on linear array images.
 *
 * Inputs:
 * - 0: Image
 *
 * Outputs:
 * - 0: Image
 *
 * @ingroup ultrasound
 */
class FAST_EXPORT  UltrasoundImageCropper : public ProcessObject {
    FAST_PROCESS_OBJECT(UltrasoundImageCropper)
    public:
        /**
         * @brief Create instance
         * @param physicalWidth The physical width of the scanning region. This is used to set the pixel spacing correctly.
         * @param staticCropping Estimate the cropping parameters (offset, size) on the first frame only, then use these for all subsequent frames
         * @param thresholdVertical Number of pixels that have to be above 0 to accept an image column to be outside of ultrasound sector
         * @param thresholdHorizontal Number of pixels that have to be above 0 to accept an image row to be outside of ultrasound sector
         * @return instance
         */
        FAST_CONSTRUCTOR(UltrasoundImageCropper,
                         float, physicalWidth, = 0,
                         bool, staticCropping, = false,
                         float, thresholdVertical, = 30,
                         float, thresholdHorizontal, = 10
        )
        /**
         * By setting the physical width of the image, the physical height can also
         * be calculated and the spacing of the image set correctly.
         *
         * @param width
         */
        void setPhysicalWidth(float width);
        /**
         * If static cropping is enabled, the cropping parameters are calculated only for the first frame.
         * Otherwise, it is recalculated for every frame.
         * @param staticCropping
         */
        void setStaticCropping(bool staticCropping);
        void setThresholds(float threshold1, float threshold2);
		void loadAttributes() override;
    private:
        void execute();

        float m_physicalWidth = 0;
        bool m_staticCropping = false;
        float m_threshold1 = 30;
        float m_threshold2 = 10;

    int m_width = -1, m_height = -1, m_offsetX = -1, m_offsetY = -1;
        Vector3f m_spacing;
};

}
