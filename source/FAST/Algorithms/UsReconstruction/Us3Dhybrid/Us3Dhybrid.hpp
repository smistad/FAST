#ifndef US_3D_HYBRID_HPP_
#define US_3D_HYBRID_HPP_

#include "FAST/ProcessObject.hpp"
#include "FAST/ExecutionDevice.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

    class Us3Dhybrid : public ProcessObject {
        FAST_OBJECT(Us3Dhybrid)
    public:
        void setOutputType(DataType type);
        ~Us3Dhybrid();
    private:
        Us3Dhybrid();
        void execute();
        void waitToFinish();

        // Control variables
        bool firstFrameSet;
        bool reachedEndOfStream;
        bool volumeCalculated;
        bool volumeInitialized;
        int iterartorCounter;
        DataType mOutputType;
        bool mOutputTypeSet;

        // Setting variables
        float dv; //voxel spacing
        float Rmax;

        // Images and volumes
        Image::pointer firstFrame;
        Vector3i volumeSize;
        Image::pointer output;
        Image::pointer AccumulationVolume;
        ImageAccess::pointer volAccess;

        // List
        std::vector<Image::pointer> frameList; //TODO finn noe med bigger kapasitet
        std::vector<Vector3f> frameMinList;
        std::vector<Vector3f> frameMaxList;
        std::vector<Vector3f> frameBaseCornerList;
        std::vector<Vector3f> framePlaneNormalList;

        // Potetial OpenCL variables
        cl::Buffer mCLMask;
        cl::Kernel mKernel;
        unsigned char mDimensionCLCodeCompiledFor;
        DataType mTypeCLCodeCompiledFor;
    };

} // end namespace fast

#endif /* US_3D_HYBRID_HPP_ */