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
        void setScaleToMax(float scaleToMax);
        void setVoxelSpacing(float voxelSpacing);
        void setDV(float dv);
        void setRmax(float maxRvalue);
        void setGlobalScaling(float globalScaling);
        void setZDirInitSpacing(float zInitSpacing);
        void setPNNrunMode(bool pnnRunMode);
        void setVNNrunMode(bool vnnRunMode);
        void setCLrun(bool clRunMode);
        void printEndStats();
        void setVerbosity(int verbosity); //TODO USE

        //Returns something
        bool hasCalculatedVolume();
        double calculateRuntime(int part);
        ~Us3Dhybrid();
    private:
        Us3Dhybrid();
        void execute();
        void waitToFinish();

        // Core functions
        void executeAlgorithm();
        void executeAlgorithmOnHost();
        void executeFramePNN(Image::pointer frame);
        void executeVNN();
        void initVolume(Image::pointer rootFrame);
        void generateOutputVolume();
        void generateOutputVolume(ExecutionDevice::pointer device);

        // Core OpenCL functions
        void recompileAlgorithmOpenCLCode();
        void recompileNormalizeOpenCLCode();

        void executeOpenCLTest();

        // Helper functions in class
        void accumulateValuesInVolume(Vector3i volumePoint, float p, float w);
        void accumulateValuesInVolumeData(Vector3i volumePoint, float p, float w);
        void addTransformationToFrame(Image::pointer frame, AffineTransformation::pointer addTransformation);
        Vector2i getFrameRangeInVolume(int frameNr, int domDir, int dir);
        AffineTransformation::pointer getInverseTransformation(Image::pointer frame);

        float getPixelValue(Vector3f point); 
        float getPixelValueData(Vector3f point);

        // Previously non-class helper functions
        Vector3f getIntersectionOfPlane(Vector3i startPoint, float distance, Vector3f normalVector);
        Vector3f getLocalIntersectionOfPlane(Vector3f intersectionPointWorld, AffineTransformation::pointer frameInverseTransform);
        bool isWithinFrame(Vector3f intersectionPointLocal, Vector3ui frameSize, float bufferXY, float bufferZ);
        float calculatePlaneDvalue(Vector3f pointInPlane, Vector3f planeNormal);
        Vector3f getBasePointInPlane(Vector3f rootPoint, Vector3f normal, float planeDvalue, int a, int b, int domDir);
        float getPointDistanceAlongNormal(Vector3i A, Vector3f B, Vector3f normal);
        float getDistanceAlongNormal(Vector3f point, Vector3f normal, Vector3f planePoint, Vector3f planeNormal);
        Vector2i getDomDirRange(Vector3f basePoint, int domDir, float dfDom, Vector3i volumeSize);
        Vector3i getVolumePointLocation(int a, int b, int c, int domDir);
        float calculateHalfWidth(float d1, float d2, float dv, float Rmax);
        int getDominatingVectorDirection(Vector3f v);
        Vector3i getRoundedIntVector3f(Vector3f v);
        Vector3f getImagePlaneNormal(Image::pointer frame);
        bool volumePointOutsideVolume(Vector3i volumePoint, Vector3i volumeSize);

        // Helper OpenCL classes
        cl_float16 transform4x4tofloat16(AffineTransformation::pointer imgTransform);

        // Control variables
        bool firstFrameSet;
        bool reachedEndOfStream;
        bool volumeCalculated;
        bool volumeInitialized;
        int iterartorCounter;
        DataType mOutputType;
        bool mOutputTypeSet;

        // Benchmark variables
        clock_t loadingStarted;
        clock_t loadingEnded;
        clock_t initVolumeStarted;
        clock_t initVolumeEnded;
        clock_t algorithmStarted;
        clock_t algorithmEnded;
        clock_t algorithmLoopStarted;
        clock_t algorithmLoopEnded;
        clock_t normalizationStarted;
        clock_t normalizationEnded;
        int verbosityLevel; //TODO USE

        // Setting variables
        float dv; //voxel spacing
        float mVoxelSpacing;
        float Rmax;
        float mScaleToMax;
        float globalScalingValue;
        bool runAsPNNonly;
        bool runAsVNNonly;
        bool runCLhybrid;
        float zDirInitSpacing;

        // Images and volumes
        Image::pointer firstFrame;
        Image::pointer outputVolume;
        Image::pointer AccumulationVolume;
        ImageAccess::pointer volAccess;
        Vector3i volumeSize;
        float * volData;
        AffineTransformation::pointer volumeTransform;
        int yLocMultiplier;
        int zLocMultiplier;
        ImageAccess::pointer frameAccess;
        Vector3ui frameSize;
        uchar * frameData;
        uint frameChannels;
        DataType frameType;

        // List
        std::vector<Image::pointer> frameList; //TODO finn noe med bigger kapasitet
        std::vector<Vector3f> frameMinList;
        std::vector<Vector3f> frameMaxList;
        std::vector<Vector3f> frameBaseCornerList;
        std::vector<Vector3f> framePlaneNormalList;
        std::vector<AffineTransformation::pointer> frameInverseTransformList;
        std::vector<float> framePlaneDValueList;

        // Potetial OpenCL variables
        cl::Kernel mKernel;
        cl::Kernel mKernelNormalize;
        unsigned char mDimensionCLCodeCompiledFor;
        DataType mTypeCLCodeCompiledFor;
        float mDvCompiledFor;
        float mRmaxCompiledFor;
        Vector3i mVolumeSizeCompiledFor;
        cl::Buffer mCLVolume;
    };

} // end namespace fast

#endif /* US_3D_HYBRID_HPP_ */