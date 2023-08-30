#pragma once

#include <FAST/Algorithms/ImageResampler/ImageResampler.hpp>

namespace fast {

/**
 * @brief Resample an image so that the the pixel spacing is equal in both directions
 *
 * This image resampler will resample the image so it has the same spacing in both directions, resulting
 * in an isotropic image. This resampler will select the target spacing from the input image.
 *
 * If you want to specify exactly which spacing to use, you should use ImageResampler instead.
 *
 * Inputs:
 * - 0: Image
 *
 * Outputs:
 * - 0: Image
 *
 * @sa ImageResampler
 */
class FAST_EXPORT IsotropicResampler : public ImageResampler {
    FAST_PROCESS_OBJECT(IsotropicResampler)
public:
    /**
     * @brief Which spacing to select
     */
    enum class SpacingSelector {
        SMALLEST = 0,   // Select smallest spacing
        LARGEST,        // Select largest spacing
        X,              // Select X spacing always
        Y,              // Select Y spacing always
        Z               // Select Z spacing always
    };
    /**
     * @brief Create instance
     * @param spacingSelector Which spacing to select from image to use for resampling. Default is smallest.
     * @param useInterpolation Whether to use linear interpolation, or just nearest neighbor.
     * @return instance
     */
    FAST_CONSTRUCTOR(IsotropicResampler,
                     SpacingSelector, spacingSelector, = SpacingSelector::SMALLEST,
                     bool, useInterpolation, = true);
    void setSpacingSelector(SpacingSelector spacingSelector);
private:
    void execute();

    SpacingSelector m_spacingSelector;
};

}
