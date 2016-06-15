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
        //TODO set functions in here to be testable
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

        // Previously non-class helper functions
        Vector3f getIntersectionOfPlane(Vector3i startPoint, float distance, Vector3f normalVector);
        Vector3f getLocalIntersectionOfPlane(Vector3f intersectionPointWorld, AffineTransformation::pointer frameInverseTransform);
        bool isWithinFrame(Vector3f intersectionPointLocal, Vector3ui frameSize);
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
        std::vector<float> framePlaneDValueList;

        // Potetial OpenCL variables
        cl::Buffer mCLMask;
        cl::Kernel mKernel;
        unsigned char mDimensionCLCodeCompiledFor;
        DataType mTypeCLCodeCompiledFor;
    };

} // end namespace fast

#endif /* US_3D_HYBRID_HPP_ */