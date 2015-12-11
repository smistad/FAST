#ifndef AVERAGEIMAGES_HPP_
#define AVERAGEIMAGES_HPP_

#include "FAST/ProcessObject.hpp"
#include "FAST/ExecutionDevice.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

    class AverageImages : public ProcessObject {
        FAST_OBJECT(AverageImages)
    public:
        void setCutOverhead(bool cut);

        ~AverageImages();
    protected:
        AverageImages();
        void execute();
        //void waitToFinish();
        void recompileOpenCLCode(Image::pointer input);

        bool cutOverhead;

        cl::Kernel mKernel;
        unsigned char mDimensionCLCodeCompiledFor;
        DataType mTypeCLCodeCompiledFor;
        DataType mOutputType;
        bool mOutputTypeSet;
    };
}

#endif

