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
 * - 0: Image float
 *
 */
class FAST_EXPORT IntensityNormalization : public ProcessObject {
    FAST_PROCESS_OBJECT(IntensityNormalization);
    public:
        /**
         * @brief Create instance
         * Every pixel value is scaled so that:
         * value = (value - minimumIntensity) / (maximumIntensity - minimumIntensity);
         * value = value*(valueHigh - valueLow) + valueLow;
         *
         * @param valueLow minimum value after normalization
         * @param valueHigh maximum value after normalization
         * @param minimumIntensity set fixed minimum intensity, if not set it is calculated for each image
         * @param maximumIntensity set fixed maxiumum intensity, if not set it is calculated for each image
         * @return instance
         */
        FAST_CONSTRUCTOR(IntensityNormalization,
                         float, valueLow, = 0.0f,
                         float, valueHigh, = 1.0f,
                         float, minimumIntensity, = std::numeric_limits<float>::quiet_NaN(),
                         float, maximumIntensity, = std::numeric_limits<float>::quiet_NaN()
        )
        void setMinimumIntensity(float intensity);
        void setMaximumIntensity(float intensity);
        void setLowestValue(float value);
        void setHighestValue(float value);
    private:
        void execute();

        float mLow, mHigh;
        float m_minIntensity, m_maxIntensity;
};

} // end namespace fast
