#ifndef FAST_KINECT_TRACKING_HPP_
#define FAST_KINECT_TRACKING_HPP_

#include "FAST/ProcessObject.hpp"

namespace fast {

class Image;
class Mesh;
class KinectStreamer;

class KinectTracking : public ProcessObject {
    FAST_OBJECT(KinectTracking)
    public:
        void addLine(Vector2i start, Vector2i end);
        void calculateTargetCloud(SharedPointer<KinectStreamer> streamer);
        void restart();
        bool toggleRecord(std::string path);
        std::string getRecordingName() const;
        uint getFramesStored() const;
        bool isRecording() const;
    private:
        KinectTracking();
        void execute();

        SharedPointer<Image> mAnnotationImage;
        SharedPointer<Mesh> mCurrentCloud;
        SharedPointer<Mesh> mTargetCloud;
        bool mTargetCloudExtracted = false;
        bool mRecording = false;
        std::string mStoragePath;
        std::string mRecordingName;
        uint mFrameCounter;
};

}

#endif
