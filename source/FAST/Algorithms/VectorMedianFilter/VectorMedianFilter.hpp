#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

class VectorMedianFilter : public ProcessObject {
    FAST_OBJECT(VectorMedianFilter)
    public:
        /**
         * Set window size of median filter. Must be odd
         * @param size
         */
        void setWindowSize(int size);
    private:
        VectorMedianFilter();
        void execute() override;

        int m_windowSize = 7;
};

}