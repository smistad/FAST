#ifndef FAST_KINECT_TRACKING_GUI_HPP_
#define FAST_KINECT_TRACKING_GUI_HPP_

#include "FAST/Visualization/Window.hpp"

class QPushButton;
class QLineEdit;
class QLabel;
class QElapsedTimer;

namespace fast {

class KinectStreamer;
class KinectTracking;

class FAST_EXPORT  KinectTrackingGUI : public Window {
    FAST_OBJECT(KinectTrackingGUI)
    public:
        void extractPointCloud();
        void restart();
        void toggleRecord();
        void updateMessages();
    private:
        KinectTrackingGUI();

        SharedPointer<KinectStreamer> mStreamer;
        SharedPointer<KinectTracking> mTracking;
        QPushButton* mRecordButton;
        QLineEdit* mStorageDir;
        QLabel* mRecordingInformation;
        QElapsedTimer* mRecordTimer;

};

}

#endif
