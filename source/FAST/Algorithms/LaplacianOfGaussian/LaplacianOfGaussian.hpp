#pragma once

#include "FAST/ProcessObject.hpp"
#include "FAST/ExecutionDevice.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

/**
 * @brief Calculate image gradient using Laplacian of Gaussian method
 *
 * @ingroup filter
 */
class FAST_EXPORT  LaplacianOfGaussian : public ProcessObject {
    FAST_PROCESS_OBJECT(LaplacianOfGaussian)
    public:
        FAST_CONSTRUCTOR(LaplacianOfGaussian,
                         float, stdDev, = 1.0f,
                         uchar, maskSize, = 0
        )
        void setMaskSize(unsigned char maskSize);
        void setStandardDeviation(float stdDev);
        ~LaplacianOfGaussian();
    private:
        void execute();
        void waitToFinish();
        void createMask(Image::pointer input, char maskSize);
        void recompileOpenCLCode(Image::pointer input);

        unsigned char mMaskSize = 0;
        float mStdDev;

        cl::Buffer mCLMask;
        std::unique_ptr<float[]> mMask;
        bool mRecreateMask;

        cl::Kernel mKernel;
        unsigned char mDimensionCLCodeCompiledFor;
        DataType mTypeCLCodeCompiledFor;

};

} // end namespace fast
