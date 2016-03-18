#ifndef FILTERING_HPP_
#define FILTERING_HPP_

#include "FAST/ProcessObject.hpp"
#include "FAST/ExecutionDevice.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

    class Filtering : public ProcessObject {
        FAST_OBJECT(Filtering)
    public:
        void setMaskSize(unsigned char maskSize);
        void setOutputType(DataType type);
        void setConvRunType(int runType);
        ~Filtering(); //virtual
    protected: //private:
        Filtering();
        void execute();
        void waitToFinish();
        virtual bool isSeparable();
        virtual float * getSeparable(int dir);
        void createMask(Image::pointer input, uchar maskSize);
        virtual float createMaskValue(int x, int y, int z = 1); //was z=0
        void recompileOpenCLCode(Image::pointer input);

        char mMaskSize;
        char mMaskDefaultSize;
        bool mIsDifferential;
        int mConvRunType; // 0 naive, 1 onepass, 2 adv..

        cl::Buffer mCLMask;
        float * mMask;
        cl::Buffer mCLMaskX;
        cl::Buffer mCLMaskY;
        float * mMaskX;
        float * mMaskY;
        bool mRecreateMask;

        cl::Kernel mKernel;
        cl::Kernel mKernelDirX;
        cl::Kernel mKernelDirY;
        unsigned char mDimensionCLCodeCompiledFor;
        DataType mTypeCLCodeCompiledFor;
        DataType mOutputType;
        bool mOutputTypeSet;
        int mConvRunTypeCompiledFor;
        char mMaskSizeCompiledFor;
        bool mNeedRebuild;
    };

} // end namespace fast

#endif /* FILTERING_HPP_ */
