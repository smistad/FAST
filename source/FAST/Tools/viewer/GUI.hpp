#ifndef FAST_VIEWER_GUI_HPP_
#define FAST_VIEWER_GUI_HPP_

#include "FAST/Visualization/Window.hpp"
#include "FAST/Pipeline.hpp"

class QPushButton;
class QLineEdit;
class QLabel;
class QElapsedTimer;
class QComboBox;
class QVBoxLayout;
class QListWidget;
class QSlider;

namespace fast {

class PipelineWidget;
class ImageFileStreamer;

class GUI : public Window {
    FAST_OBJECT(GUI)
    public:
        void selectPipeline();
        void editPipeline();
        void newPipeline();
        void addInputData();
        void selectInputData();
        void playPause();
        void increaseTimestep();
        void setTimestep();
    private:
        GUI();
        void refreshStreams();

        SharedPointer<ImageFileStreamer> mStreamer;
        std::vector<std::string> mStreamNames;
        std::vector<Pipeline> mPipelines;

        QListWidget* mList;
        QComboBox* mSelectPipeline;
        QPushButton* mPlayPauseButton;
        QVBoxLayout* menuLayout;
        QSlider* mTimestepSlider;
        int menuWidth;
        PipelineWidget* mPipelineWidget;
};

} // end namespace fast

#endif
