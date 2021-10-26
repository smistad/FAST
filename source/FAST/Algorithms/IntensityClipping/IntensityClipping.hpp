#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

/**
 * @brief Clips the pixel intensities of an image to specific values
 *
 * Inputs:
 * - 0: Image
 *
 * Outputs:
 * - 0: Image
 *
 * @ingroup filter
 */
class FAST_EXPORT IntensityClipping : public ProcessObject {
    FAST_PROCESS_OBJECT(IntensityClipping)
    public:
        /**
         * @brief Create instance
         *
         * Clips the pixel intensities to the provided min and max values.
         *
         * @param minValue
         * @param maxValue
         * @return instance
         */
        FAST_CONSTRUCTOR(IntensityClipping,
                         float, minValue, = 0.0f,
                         float, maxValue, = 1.0f
        )
        void setMinValue(float value);
        void setMaxValue(float value);
    private:
        void execute() override;

        float m_min, m_max;
};
}