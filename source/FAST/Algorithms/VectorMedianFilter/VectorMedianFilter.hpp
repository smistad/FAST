#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

class FAST_EXPORT VectorMedianFilter : public ProcessObject {
    FAST_OBJECT(VectorMedianFilter)
    public:
        /**
         * Set window size of median filter. Must be odd
         * @param size
         */
        void setWindowSize(int size);
        void loadAttributes() override;
    private:
        VectorMedianFilter();
        void execute() override;

        int m_windowSize = 7;
};

}