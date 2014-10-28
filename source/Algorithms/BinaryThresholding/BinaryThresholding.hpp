#ifndef BINARY_THRESHOLDING_HPP
#define BINARY_THRESHOLDING_HPP

#include "ProcessObject.hpp"
#include "Image.hpp"

namespace fast {

class BinaryThresholding : public ProcessObject {
    FAST_OBJECT(BinaryThresholding)
    public:
        void setInput(ImageData::pointer input);
        void setDevice(ExecutionDevice::pointer device);
        ImageData::pointer getOutput();
        void setLowerThreshold(float threshold);
        void setUpperThreshold(float threshold);
    private:
        BinaryThresholding();
        void execute();
        void waitToFinish();

        ImageData::pointer mInput;
        ImageData::pointer mOutput;
        ExecutionDevice::pointer mDevice;
        float mLowerThreshold;
        float mUpperThreshold;
        bool mLowerThresholdSet;
        bool mUpperThresholdSet;
};

}

#endif
