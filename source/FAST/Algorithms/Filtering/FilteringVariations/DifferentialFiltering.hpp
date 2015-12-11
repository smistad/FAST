#ifndef DIFF_FILTERING_HPP_
#define DIFF_FILTERING_HPP_

#include "FAST/ProcessObject.hpp"
#include "FAST/Algorithms/Filtering/Filtering.hpp"
#include "FAST/ExecutionDevice.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

    class DifferentialFiltering : public Filtering {
        FAST_OBJECT(DifferentialFiltering)
    public:
        //void setMaskSize(unsigned char maskSize); //Is always 3
        //void setDirection(int direction);
        //float createMaskValue(int x, int y, int z = 1); //Special for sobel, if no z, use z=1
        //void setOutputType(DataType type);
        int mDifferentialAddValue;
        void createMask(Image::pointer input, uchar maskSize);
        ~DifferentialFiltering();
    protected:
        DifferentialFiltering();
        //void execute();
        //bool isSeparable(); //or maybe?
        //void waitToFinish();
        
        //float createMaskValue(int x, int y, int z = 1); //Special for sobel, if no z, use z=1
        //void recompileOpenCLCode(Image::pointer input);

        //char mMaskSize;
        //float mStdDev;
        //int mDirection;

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

#endif /* SOBEL_FILTERING_HPP_ */