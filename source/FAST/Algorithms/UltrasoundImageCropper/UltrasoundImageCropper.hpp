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
    private:
        UltrasoundImageCropper();
        void execute();

        float m_physicalWidth = 0;
};

}

#endif
