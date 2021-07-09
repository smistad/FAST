#pragma once

#include "FAST/ProcessObject.hpp"

namespace fast {

class FAST_EXPORT  ImageCropper : public ProcessObject {
    FAST_PROCESS_OBJECT(ImageCropper)
    public:
        FAST_CONSTRUCTOR(ImageCropper,
                         VectorXi, size,,
                         VectorXi, offset, = Vector3i::Zero()
        )
        FAST_CONSTRUCTOR(ImageCropper,
                         float, cropBottom, = -1.0f,
                         float, cropTop, = -1.0f
        )
        void setOffset(VectorXi offset);
        void setSize(VectorXi size);
        /**
         * If out of bounds cropping is allowed, offset can be negative and size can be larger than image.
         * Any pixels outside of image will be replaced with 0.
         */
        void allowOutOfBoundsCropping(bool allow);

        void setCropBottom(float fraction);
        void setCropTop(float fraction);
    private:
        void execute();
        void init();

        float mCropBottom;
        float mCropTop;
        VectorXi mOffset;
        VectorXi mSize;
        bool mAllowOutOfBoundsCropping;
};


} // end namespace fast
