#pragma once

#include "FAST/ProcessObject.hpp"
#include "FAST/ExecutionDevice.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

/**
 * @brief Smoothing by convolution with a Gaussian mask
 *
 * Inputs:
 * - 0: Image, 2D or 3D
 *
 * Outputs:
 * - 0: Image, 2D or 3D
 *
 * @ingroup filter
 */
class FAST_EXPORT GaussianSmoothing : public ProcessObject {
    FAST_PROCESS_OBJECT(GaussianSmoothing)
    public:
        /**
         * @brief Create instance
         * @param stdDev Standard deviation of convolution kernel
         * @param maskSize Size of convolution filter/mask. Must be odd.
         *      If 0 filter size is determined automatically from standard deviation
         * @return instance
         */
        FAST_CONSTRUCTOR(GaussianSmoothing,
                         float, stdDev, = 0.5f,
                         uchar, maskSize, = 0
        );
        void setMaskSize(unsigned char maskSize);
        void setStandardDeviation(float stdDev);
        void setOutputType(DataType type);
        void loadAttributes() override;
        ~GaussianSmoothing();
    protected:
        void execute();
        void waitToFinish();
        void createMask(Image::pointer input, uchar maskSize, bool useSeperableFilter);
        void recompileOpenCLCode(Image::pointer input);

        char mMaskSize;
        float mStdDev;

        cl::Buffer mCLMask;
        std::unique_ptr<float[]> mMask;
        bool mRecreateMask;

        cl::Kernel mKernel;
        unsigned char mDimensionCLCodeCompiledFor;
        DataType mTypeCLCodeCompiledFor;
        DataType mOutputType;
        bool mOutputTypeSet;

};

} // end namespace fast

