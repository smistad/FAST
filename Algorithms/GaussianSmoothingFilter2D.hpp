#ifndef GAUSSIANSMOOTHINGFILTER_HPP_
#define GAUSSIANSMOOTHINGFILTER_HPP_

#include "PipelineObject.hpp"
#include "ExecutionDevice.hpp"
#include "Image2D.hpp"
#include "Image2Dt.hpp"

namespace fast {

class GaussianSmoothingFilter2D : public PipelineObject {
    FAST_OBJECT(GaussianSmoothingFilter2D)
    public:
        void setInput(Image2D::pointer input);
        void setInput(Image2Dt::pointer input);
        void setDevice(ExecutionDevice::pointer device);
        void setMaskSize(unsigned char maskSize);
        void setStandardDeviation(float stdDev);
        ImageData::pointer getOutput();
    private:
        GaussianSmoothingFilter2D();
        void execute();
        float * createMask();

        Image2D::pointer mStaticInput;
        Image2Dt::pointer mDynamicInput;
        WeakPointer<Image2D> mOutput;
        Image2D::pointer mTempOutput;
        ExecutionDevice::pointer mDevice;
        unsigned char mMaskSize;
        float mStdDev;
};

} // end namespace fast




#endif /* GAUSSIANSMOOTHINGFILTER_HPP_ */
