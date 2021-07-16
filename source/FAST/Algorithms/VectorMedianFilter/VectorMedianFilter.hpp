#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

/**
 * @brief Apply median filter on vector field to reduce noise
 *
 * Inputs:
 * - 0: Image vector field
 *
 * Outputs:
 * - 0: Image vector field
 *
 * @ingroup filter
 */
class FAST_EXPORT VectorMedianFilter : public ProcessObject {
    FAST_PROCESS_OBJECT(VectorMedianFilter)
    public:
        FAST_CONSTRUCTOR(VectorMedianFilter, int, size, = 7)
        /**
         * Set window size of median filter. Must be odd
         * @param size
         */
        void setWindowSize(int size);
        void loadAttributes() override;
    private:
        void execute() override;

        int m_windowSize = 7;
};

}