#ifndef FAST_KINECT_TRACKING_HPP_
#define FAST_KINECT_TRACKING_HPP_

#include "FAST/ProcessObject.hpp"

namespace fast {

class Image;

class KinectTracking : public ProcessObject {
    FAST_OBJECT(KinectTracking)
    public:
        void addLine(Vector2i start, Vector2i end);
    private:
        KinectTracking();
        void execute();

        SharedPointer<Image> mAnnotationImage;
};

}

#endif
