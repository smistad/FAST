#ifndef IMAGE_RESAMPLER_HPP_
#define IMAGE_RESAMPLER_HPP_

#include "FAST/ProcessObject.hpp"

namespace fast {

class FAST_EXPORT  ImageResampler : public ProcessObject {
    FAST_OBJECT(ImageResampler)
public:
    void setOutputSpacing(float spacingX, float spacingY);
    void setOutputSpacing(float spacingX, float spacingY, float spacingZ);
private:
    ImageResampler();
    void execute();

    Vector3f mSpacing;
};

}

#endif