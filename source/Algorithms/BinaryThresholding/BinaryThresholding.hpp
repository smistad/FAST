#ifndef BINARY_THRESHOLDING_HPP
#define BINARY_THRESHOLDING_HPP

#include "ProcessObject.hpp"

namespace fast {

class BinaryThresholding : public ProcessObject {
    FAST_OBJECT(BinaryThresholding)
    public:
        void setLowerThreshold(float threshold);
        void setUpperThreshold(float threshold);
    private:
        BinaryThresholding();
        void execute();
        void waitToFinish();

        float mLowerThreshold;
        float mUpperThreshold;
        bool mLowerThresholdSet;
        bool mUpperThresholdSet;
};

}

#endif
