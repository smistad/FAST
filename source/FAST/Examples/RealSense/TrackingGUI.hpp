#pragma once

#include "FAST/Visualization/Window.hpp"

class QPushButton;
class QLineEdit;
class QLabel;
class QElapsedTimer;
class QListWidget;

namespace fast {

class RealSenseStreamer;
class Tracking;

class TrackingGUI : public Window {
    FAST_OBJECT(TrackingGUI)
    public:
        void extractPointCloud();
        void restart();
        void toggleRecord();
        void updateMessages();
        void playRecording();
        void refreshRecordingsList();
    private:
        TrackingGUI();

        std::shared_ptr<RealSenseStreamer> mStreamer;
        std::shared_ptr<Tracking> mTracking;
        QPushButton* mRecordButton;
        QPushButton* mPlayButton;
        QLineEdit* mStorageDir;
        QLineEdit* mRecordingNameLineEdit;
        QLabel* mRecordingInformation;
        QElapsedTimer* mRecordTimer;
        QListWidget* mRecordingsList;

        bool mRecording = false;
        bool mPlaying = false;
        std::string mRecordingName;

};

}
