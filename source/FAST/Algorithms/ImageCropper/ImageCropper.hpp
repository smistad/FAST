#pragma once

#include "FAST/ProcessObject.hpp"

namespace fast {

/**
 * @brief Process object from cropping an image
 *
 * Uses Image::crop
 */
class FAST_EXPORT ImageCropper : public ProcessObject {
    FAST_PROCESS_OBJECT(ImageCropper)
    public:
#ifndef SWIG
        /**
         * @brief Create instance
         * @param size Cropping size
         * @param offset Offset for cropping
         * @return instance
         */
        FAST_CONSTRUCTOR(ImageCropper,
                         VectorXi, size,,
                         VectorXi, offset, = Vector3i::Zero()
        );
        /**
         * @brief Create instance
         * @param cropBottom Percent of bottom to remove
         * @param cropTop Percent of top to remove
         * @return instance
         */
        FAST_CONSTRUCTOR(ImageCropper,
                         float, cropBottom, = -1.0f,
                         float, cropTop, = -1.0f
        );
#endif
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
#ifdef SWIG
        // Avoid SWIG using constructor
        ImageCropper();
#endif
        void execute();
        void init();

        float mCropBottom;
        float mCropTop;
        VectorXi mOffset;
        VectorXi mSize;
        bool mAllowOutOfBoundsCropping;
};

#ifdef SWIG
%extend ImageCropper {
%pythoncode %{

@classmethod
def create(cls, size=None, offset=None, cropBottom: Optional[float]=None, cropTop: Optional[float]=None):
    instance = cls()
    if cropBottom is not None:
        instance.setCropBottom(cropBottom)
    elif cropTop is not None:
        instance.setCropTop(cropTop)
    else:
        instance.setSize(size)
        instance.setOffset(offset)

    return instance

%}
}
#endif

} // end namespace fast
