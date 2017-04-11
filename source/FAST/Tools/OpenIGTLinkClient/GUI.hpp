#ifndef FAST_OPENIGTLINKCLIENT_GUI_HPP_
#define FAST_OPENIGTLINKCLIENT_GUI_HPP_

#include "FAST/Visualization/Window.hpp"

class QPushButton;
class QLineEdit;

namespace fast {

class IGTLinkStreamer;

class GUI : public Window {
    FAST_OBJECT(GUI)
    public:
        void record();
        void connect();
    private:
        GUI();

        QPushButton* connectButton;
        QLineEdit* address;
        QLineEdit* port;
        bool mConnected;
        SharedPointer<IGTLinkStreamer> mStreamer;

        QPushButton* recordButton;
        QLineEdit* storageDir;
        bool mRecording;
};

} // end namespace fast

#endif
