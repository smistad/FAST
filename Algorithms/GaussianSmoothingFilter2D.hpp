#ifndef GAUSSIANSMOOTHINGFILTER_HPP_
#define GAUSSIANSMOOTHINGFILTER_HPP_

#include "ProcessObject.hpp"
#include "ExecutionDevice.hpp"
#include "ImageData.hpp"

namespace fast {

class GaussianSmoothingFilter2D : public ProcessObject {
    FAST_OBJECT(GaussianSmoothingFilter2D)
    public:
        void setInput(ImageData::pointer input);
        void setDevice(ExecutionDevice::pointer device);
        void setMaskSize(unsigned char maskSize);
        void setStandardDeviation(float stdDev);
        ImageData::pointer getOutput();
    private:
        GaussianSmoothingFilter2D();
        void execute();
        float * createMask();

        ImageData::pointer mInput;
        WeakPointer<ImageData> mOutput;
        ImageData::pointer mTempOutput;
        ExecutionDevice::pointer mDevice;
        unsigned char mMaskSize;
        float mStdDev;
};

} // end namespace fast




#endif /* GAUSSIANSMOOTHINGFILTER_HPP_ */
