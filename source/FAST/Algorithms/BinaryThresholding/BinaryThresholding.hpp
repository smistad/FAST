#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

class FAST_EXPORT BinaryThresholding : public ProcessObject {
    FAST_OBJECT(BinaryThresholding)
    public:
        void setLowerThreshold(float threshold);
        void setUpperThreshold(float threshold);
        void loadAttributes() override;
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
