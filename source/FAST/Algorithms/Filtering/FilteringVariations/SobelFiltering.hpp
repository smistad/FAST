#ifndef SOBEL_FILTERING_HPP_
#define SOBEL_FILTERING_HPP_

#include "FAST/ProcessObject.hpp"
#include "FAST/Algorithms/Filtering/Filtering.hpp"
//#include "FAST/Algorithms/Filtering/FilteringVariations/DifferentialFiltering.hpp"
#include "FAST/ExecutionDevice.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

    class SobelFiltering : public Filtering {
        FAST_OBJECT(SobelFiltering)
    public:
        //void setMaskSize(unsigned char maskSize); //Is always 3
        void setDirection(int direction);
        float createMaskValue(int x, int y, int z = 1); //Special for sobel, if no z, use z=1
        //void setOutputType(DataType type);
        ~SobelFiltering();
    protected:
        SobelFiltering();
        //void execute();
        bool isSeparable();
        float * getSeparable(int dir);
        //void waitToFinish();
        //void createMask(Image::pointer input, uchar maskSize);
        //float createMaskValue(int x, int y, int z = 1); //Special for sobel, if no z, use z=1
        //void recompileOpenCLCode(Image::pointer input);

        //char mMaskSize;
        //float mStdDev;
        int mDirection;

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