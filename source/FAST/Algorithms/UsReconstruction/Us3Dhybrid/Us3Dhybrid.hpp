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

        // Core functions
        void executeAlgorithmOnHost();
        void initVolume(Image::pointer rootFrame);

        // Helper functions in class
        void accumulateValuesInVolume(Vector3i volumePoint, float p, float w);
        void addTransformationToFrame(Image::pointer frame, AffineTransformation::pointer addTransformation);
        Vector2i getFrameRangeInVolume(int frameNr, int domDir, int dir);
        AffineTransformation::pointer getInverseTransformation(Image::pointer frame);

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
        std::vector<AffineTransformation::pointer> frameInverseTransformList;

        // Potetial OpenCL variables
        cl::Buffer mCLMask;
        cl::Kernel mKernel;
        unsigned char mDimensionCLCodeCompiledFor;
        DataType mTypeCLCodeCompiledFor;
    };

} // end namespace fast

#endif /* US_3D_HYBRID_HPP_ */