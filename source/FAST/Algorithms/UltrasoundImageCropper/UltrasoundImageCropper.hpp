#pragma once

#include "FAST/ProcessObject.hpp"

namespace fast {

/**
 * @brief Automatic cropping of an ultrasound scanner image
 *
 * When streaming or storing images/video on ultrasound scanner, the menu and a lot of information around the actual
 * ultrasound image is included.
 * This algorithm tries to extract the ultrasound image from the entire scanner GUI image using two thresholds.
 * It does this by counting the number of non-zero pixels in every row and column of the image.
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
         *
         * @param physicalWidth The physical width of the scanning region. This is used to set the pixel spacing correctly.
         * @param staticCropping Estimate the cropping parameters (offset, size) on the first frame only, then use these for all subsequent frames
         * @param thresholdVertical
         * @param thresholdHorizontal
         * @return
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
