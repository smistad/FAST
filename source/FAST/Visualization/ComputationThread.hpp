#pragma once

#include "FAST/Object.hpp"
#include "FAST/DataChannels/DataChannel.hpp"
#include <QThread>
#include <mutex>
#include <condition_variable>
#include <vector>


namespace fast {

class View;

class FAST_EXPORT  ComputationThread : public QObject, public Object {
    Q_OBJECT
    public:
        ComputationThread(QThread* mainThread, StreamingMode mode);
        ~ComputationThread();
        bool isRunning();
        void stop();
        void addView(View* view);
        void clearViews();
        StreamingMode getStreamingMode();
        void setStreamingMode(StreamingMode mode);
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

        bool mStop = false;
        StreamingMode mStreamingMode;
};

}
