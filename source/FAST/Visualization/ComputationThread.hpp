#pragma once

#include "FAST/Object.hpp"
#include "FAST/DataChannels/DataChannel.hpp"
#include <QThread>
#include <mutex>
#include <condition_variable>
#include <vector>


namespace fast {

class Window;

class FAST_EXPORT  ComputationThread : public QObject, public Object {
    Q_OBJECT
    public:
        ComputationThread(QThread* mainThread);
        ~ComputationThread();
        bool isRunning();
        void stop();
        void setWindow(std::weak_ptr<Window> window);
    public slots:
        void run();
    signals:
        void finished();
    private:

        bool mIsRunning;
        std::condition_variable mUpdateThreadConditionVariable;
        std::mutex mUpdateThreadMutex;

        QThread* mMainThread;

        std::weak_ptr<Window> m_window;

        bool mStop = false;
};

}
