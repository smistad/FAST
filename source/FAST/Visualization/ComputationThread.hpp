#pragma once

#include "FAST/Object.hpp"
#include "FAST/DataChannels/DataChannel.hpp"
#include <QThread>
#include <mutex>
#include <condition_variable>
#include <vector>


namespace fast {

class View;
class ProcessObject;

class FAST_EXPORT  ComputationThread : public QObject, public Object {
    Q_OBJECT
    public:
        ComputationThread(QThread* mainThread);
        ~ComputationThread();
        bool isRunning();
        void stop();
        void addView(View* view);
        void clearViews();
        void setProcessObjects(std::vector<SharedPointer<ProcessObject>> processObjects);
    public slots:
        void run();
    signals:
        void finished();
    private:

        bool mIsRunning;
        std::condition_variable mUpdateThreadConditionVariable;
        std::mutex mUpdateThreadMutex;

        QThread* mMainThread;

        std::vector<View*> mViews;
        std::vector<SharedPointer<ProcessObject>> m_processObjects;

        bool mStop = false;
};

}
