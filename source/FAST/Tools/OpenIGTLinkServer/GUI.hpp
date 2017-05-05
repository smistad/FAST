#ifndef FAST_OPENIGTLINKSERVER_GUI_HPP_
#define FAST_OPENIGTLINKSERVER_GUI_HPP_

#include "FAST/Visualization/Window.hpp"
#include <igtl/igtlServerSocket.h>
#include <thread>


class QPushButton;
class QLabel;
class QListWidget;

namespace fast {

class GUI : public Window {
    FAST_OBJECT(GUI)
    public:
        void toggleServer();
        void setFilenameFormats(std::vector<std::string> formats);
        void addRecording();
        void removeRecordings();
        ~GUI();
    private:
        GUI();
        void streamData();

        std::vector<std::string> mFilenameFormats;
        QPushButton* mStartStopButton;
        QLabel* mStatus;
        QListWidget* mList;
        bool mRunning;
        int mPort;
        bool mStop;
        int mFPS;
        std::thread* mThread;
        igtl::ServerSocket::Pointer mServerSocket;

};

}

#endif
