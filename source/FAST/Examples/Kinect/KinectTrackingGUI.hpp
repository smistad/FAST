#ifndef FAST_KINECT_TRACKING_GUI_HPP_
#define FAST_KINECT_TRACKING_GUI_HPP_

#include "FAST/Visualization/Window.hpp"

namespace fast {

class KinectStreamer;
class KinectTracking;

class KinectTrackingGUI : public Window {
    FAST_OBJECT(KinectTrackingGUI)
    public:
        void extractPointCloud();
        void restart();
    private:
        KinectTrackingGUI();

        SharedPointer<KinectStreamer> mStreamer;
        SharedPointer<KinectTracking> mTracking;

};

}

#endif
