#ifndef ULTRASOUND_IMAGE_CROPPER_HPP_
#define ULTRASOUND_IMAGE_CROPPER_HPP_

#include "FAST/ProcessObject.hpp"

namespace fast {

class FAST_EXPORT  UltrasoundImageCropper : public ProcessObject {
    FAST_OBJECT(UltrasoundImageCropper)
    public:
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
    private:
        UltrasoundImageCropper();
        void execute();

        float m_physicalWidth = 0;
        bool m_staticCropping = false;

        int m_width = -1, m_height = -1, m_offsetX = -1, m_offsetY = -1;
        Vector3f m_spacing;
};

}

#endif
