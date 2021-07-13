#pragma once

#include "FAST/ProcessObject.hpp"

namespace fast {

/**
 * @brief Normalize intensities of an image to a 0-1 or custom range.
 *
 * This process object will scale the pixel values of an
 * image to a value between 0 and 1 (default) or other
 * values if set.
 *
 * Inputs:
 * - 0: Image
 *
 * Outputs:
 * - 0: Image
 *
 */
class FAST_EXPORT IntensityNormalization : public ProcessObject {
    FAST_PROCESS_OBJECT(IntensityNormalization);
    public:
        FAST_CONSTRUCTOR(IntensityNormalization,
                         float, valueLow, = 0.0f,
                         float, valueHigh, = 1.0f)
        void setLowestValue(float value);
        void setHighestValue(float value);
    private:
        void execute();

        float mLow, mHigh;
};

} // end namespace fast
