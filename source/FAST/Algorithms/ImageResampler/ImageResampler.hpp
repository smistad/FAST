#pragma once

#include "FAST/ProcessObject.hpp"

namespace fast {

/**
 * @brief Resample an image to a given spatial resolution
 */
class FAST_EXPORT  ImageResampler : public ProcessObject {
    FAST_PROCESS_OBJECT(ImageResampler)
public:
    /**
     * @brief Create instance
     * @param spacingX Pixel spacing in x direction
     * @param spacingY Pixel spacing in y direction
     * @param spacingZ Pixel spacing in z direction
     * @param useInterpolation Whether to use linear interpolation, or just nearest neighbor.
     * @return instance
     */
    FAST_CONSTRUCTOR(ImageResampler,
                     float, spacingX,,
                     float, spacingY,,
                     float, spacingZ, = -1.0f,
                     bool, useInterpolation, = true);
    void setOutputSpacing(float spacingX, float spacingY);
    void setOutputSpacing(float spacingX, float spacingY, float spacingZ);
    void setInterpolation(bool useInterpolation);
    void loadAttributes();
private:
    ImageResampler();
    void execute();

    Vector3f mSpacing;
    bool mInterpolationSet, mInterpolation;
};

}
