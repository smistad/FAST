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

namespace fast {

class PipelineWidget;
class ImageFileStreamer;

class GUI : public Window {
    FAST_OBJECT(GUI)
    public:
        void selectPipeline();
        void editPipeline();
        void addInputData();
        void selectInputData();
    private:
        GUI();
        void refreshStreams();

        SharedPointer<ImageFileStreamer> mStreamer;
        std::vector<std::string> mStreamNames;
        std::vector<Pipeline> mPipelines;

        QListWidget* mList;
        QComboBox* mSelectPipeline;
        QVBoxLayout* menuLayout;
        int menuWidth;
        PipelineWidget* mPipelineWidget;
};

} // end namespace fast

#endif
