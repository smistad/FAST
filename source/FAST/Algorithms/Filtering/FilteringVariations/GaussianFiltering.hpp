#ifndef GAUSSIAN_FILTERING_HPP_
#define GAUSSIAN_FILTERING_HPP_

#include "FAST/ProcessObject.hpp"
#include "FAST/Algorithms/Filtering/Filtering.hpp"
#include "FAST/ExecutionDevice.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

    class GaussianFiltering : public Filtering {
        FAST_OBJECT(GaussianFiltering)
    public:
        //void setMaskSize(unsigned char maskSize);
        void setStandardDeviation(float stdDev);
        //void setOutputType(DataType type);
        ~GaussianFiltering();
    protected:
        GaussianFiltering();
        //void execute();
        bool isSeparable();
        float * getSeparable(int dir);
        //void waitToFinish();
        //void createMask(Image::pointer input, uchar maskSize);
        float createMaskValue(int x, int y, int z = 0);
        //void recompileOpenCLCode(Image::pointer input);

        //char mMaskSize;
        float mStdDev;

        //cl::Buffer mCLMask;
        //float * mMask;
        //bool mRecreateMask;

        //cl::Kernel mKernel;
        //unsigned char mDimensionCLCodeCompiledFor;
        //DataType mTypeCLCodeCompiledFor;
        //DataType mOutputType;
        //bool mOutputTypeSet;

    };

} // end namespace fast

#endif /* GAUSSIAN_FILTERING_HPP_ */