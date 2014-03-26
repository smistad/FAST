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
        void setInput(Image2D::Ptr input);
        void setInput(Image2Dt::Ptr input);
        void setDevice(ExecutionDevice::Ptr device);
        void setMaskSize(unsigned char maskSize);
        void setStandardDeviation(float stdDev);
        Image2D::Ptr getOutput();
    private:
        GaussianSmoothingFilter2D();
        void execute();
        float * createMask();

        Image2D::Ptr mStaticInput;
        Image2Dt::Ptr mDynamicInput;
        WeakPointer<Image2D> mOutput;
        Image2D::Ptr mTempOutput;
        ExecutionDevice::Ptr mDevice;
        unsigned char mMaskSize;
        float mStdDev;
};

} // end namespace fast




#endif /* GAUSSIANSMOOTHINGFILTER_HPP_ */
