#ifndef FAST_OPENIGTLINKCLIENT_GUI_HPP_
#define FAST_OPENIGTLINKCLIENT_GUI_HPP_

#include "FAST/Visualization/Window.hpp"
#include "FAST/Pipeline.hpp"

class QPushButton;
class QLineEdit;
class QLabel;
class QElapsedTimer;
class QComboBox;
class QVBoxLayout;

namespace fast {

class IGTLinkStreamer;
class OpenIGTLinkClient;
class PipelineWidget;

class GUI : public Window {
    FAST_OBJECT(GUI)
    public:
        void record();
        void connect();
        void selectStream();
        void selectPipeline();
        void editPipeline();
        void updateMessages();
    private:
        GUI();
        void refreshStreams();

        QPushButton* connectButton;
        QLineEdit* mAddress;
        QLineEdit* mPort;
        bool mConnected;
        SharedPointer<IGTLinkStreamer> mStreamer;
        std::vector<std::string> mStreamNames;
        std::vector<Pipeline> mPipelines;
        SharedPointer<OpenIGTLinkClient> mClient;

        QLabel* recordingInformation;
        QPushButton* recordButton;
        QElapsedTimer* mRecordTimer;
        QLineEdit* storageDir;
        QComboBox* mSelectStream;
        QComboBox* mSelectPipeline;
        QVBoxLayout* menuLayout;
        int menuWidth;
        PipelineWidget* mPipelineWidget;
};

} // end namespace fast

#endif
