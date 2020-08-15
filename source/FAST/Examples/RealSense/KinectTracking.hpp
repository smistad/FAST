#ifndef FAST_KINECT_TRACKING_HPP_
#define FAST_KINECT_TRACKING_HPP_

#include "FAST/ProcessObject.hpp"

namespace fast {

class Image;
class Mesh;
class RealSenseStreamer;

class KinectTracking : public ProcessObject {
    FAST_OBJECT(KinectTracking)
    public:
        void addLine(Vector2i start, Vector2i end);
        void calculateTargetCloud(std::shared_ptr<RealSenseStreamer> streamer);
        void restart();
        void startRecording(std::string path);
        void stopRecording();
        void setTargetCloud(std::shared_ptr<Mesh> target);
        std::shared_ptr<Mesh> getTargetCloud() const;
        uint getFramesStored() const;
        bool isRecording() const;
    private:
        KinectTracking();
        void execute();

        std::shared_ptr<Image> mAnnotationImage;
        std::shared_ptr<Mesh> mCurrentCloud;
        std::shared_ptr<Mesh> mTargetCloud;
        bool mTargetCloudExtracted = false;
        bool mRecording = false;
        std::string mStoragePath;
        std::string mRecordingName;
        uint mFrameCounter;
};

}

#endif
