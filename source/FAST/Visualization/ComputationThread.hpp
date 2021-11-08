#pragma once

#include "FAST/Object.hpp"
#include "FAST/DataChannels/DataChannel.hpp"
#include <QThread>
#include <mutex>
#include <condition_variable>
#include <vector>


namespace fast {

class ProcessObject;
class View;

class FAST_EXPORT  ComputationThread : public QObject, public Object {
    Q_OBJECT
    FAST_OBJECT_V4(ComputationThread)
    public:
        FAST_CONSTRUCTOR(ComputationThread)
        ~ComputationThread();
        bool isRunning();
        void stop();
        void addView(View* view);
        void clearViews();
        View* getView(int index) const;
        std::vector<View*> getViews() const;
        void addProcessObject(std::shared_ptr<ProcessObject> po);
        void clearProcessObjects();
        std::vector<std::shared_ptr<ProcessObject>> getProcessObjects() const;
        std::shared_ptr<ProcessObject> getProcessObjects(int index) const;
        void start();
    public Q_SLOTS:
        void run();
    Q_SIGNALS:
        void finished();
    private:

        bool mIsRunning;
        std::condition_variable mUpdateThreadConditionVariable;
        std::mutex mUpdateThreadMutex;

        std::vector<View*> m_views;
        std::vector<std::shared_ptr<ProcessObject>> m_processObjects;

        bool mStop = false;
};

}
