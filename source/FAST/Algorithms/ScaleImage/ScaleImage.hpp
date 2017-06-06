#ifndef SCALE_IMAGE_HPP
#define SCALE_IMAGE_HPP

#include "FAST/ProcessObject.hpp"

namespace fast {

/**
 * This process object will scale the pixel values of an
 * image to a value between 0 and 1 (default) or other
 * values if set.
 */
class FAST_EXPORT  ScaleImage : public ProcessObject {
    FAST_OBJECT(ScaleImage);
    public:
        void setLowestValue(float value);
        void setHighestValue(float value);
    private:
        ScaleImage();
        void execute();

        float mLow, mHigh;
};

} // end namespace fast

#endif
