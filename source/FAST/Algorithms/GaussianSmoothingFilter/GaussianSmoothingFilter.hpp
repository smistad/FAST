#ifndef GAUSSIANSMOOTHINGFILTER_HPP_
#define GAUSSIANSMOOTHINGFILTER_HPP_

#include "FAST/ProcessObject.hpp"
#include "FAST/ExecutionDevice.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

class FAST_EXPORT  GaussianSmoothingFilter : public ProcessObject {
    FAST_OBJECT(GaussianSmoothingFilter)
    public:
        void setMaskSize(unsigned char maskSize);
        void setStandardDeviation(float stdDev);
        void setOutputType(DataType type);
        ~GaussianSmoothingFilter();
    private:
        GaussianSmoothingFilter();
        void execute();
        void waitToFinish();
        void createMask(Image::pointer input, uchar maskSize, bool useSeperableFilter);
        void recompileOpenCLCode(Image::pointer input);

        char mMaskSize;
        float mStdDev;

        cl::Buffer mCLMask;
        float * mMask;
        bool mRecreateMask;

        cl::Kernel mKernel;
        unsigned char mDimensionCLCodeCompiledFor;
        DataType mTypeCLCodeCompiledFor;
        DataType mOutputType;
        bool mOutputTypeSet;

};

} // end namespace fast




#endif /* GAUSSIANSMOOTHINGFILTER_HPP_ */
