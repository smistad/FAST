#ifndef LAPLACIAN_OF_GAUSSIAN_HPP_
#define LAPLACIAN_OF_GAUSSIAN_HPP_

#include "FAST/ProcessObject.hpp"
#include "FAST/ExecutionDevice.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

class FAST_EXPORT  LaplacianOfGaussian : public ProcessObject {
    FAST_OBJECT(LaplacianOfGaussian)
    public:
        void setMaskSize(unsigned char maskSize);
        void setStandardDeviation(float stdDev);
        ~LaplacianOfGaussian();
    private:
        LaplacianOfGaussian();
        void execute();
        void waitToFinish();
        void createMask(Image::pointer input);
        void recompileOpenCLCode(Image::pointer input);

        unsigned char mMaskSize;
        float mStdDev;

        cl::Buffer mCLMask;
        float * mMask;
        bool mRecreateMask;

        cl::Kernel mKernel;
        unsigned char mDimensionCLCodeCompiledFor;
        DataType mTypeCLCodeCompiledFor;

};

} // end namespace fast




#endif
