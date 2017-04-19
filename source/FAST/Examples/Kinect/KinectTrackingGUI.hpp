#ifndef FAST_KINECT_TRACKING_GUI_HPP_
#define FAST_KINECT_TRACKING_GUI_HPP_

#include "FAST/Visualization/Window.hpp"

namespace fast {

class KinectTrackingGUI : public Window {
    FAST_OBJECT(KinectTrackingGUI)
    public:
        void mouseMoveEvent(QMouseEvent* event);
    private:
        KinectTrackingGUI();

};

}

#endif
