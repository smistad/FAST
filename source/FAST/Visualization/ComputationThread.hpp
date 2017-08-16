#ifndef COMPUTATION_THREAD_HPP
#define COMPUTATION_THREAD_HPP

#include "FAST/Object.hpp"
#include "FAST/DataPort.hpp"
#include <QThread>
#include <mutex>
#include <condition_variable>
#include <vector>


namespace fast {

class View;

class FAST_EXPORT  ComputationThread : public QObject, public Object {
    Q_OBJECT
    public:
        ComputationThread(QThread* mainThread);
        ~ComputationThread();
        bool isRunning();
        void stop();
        void addView(View* view);
        void clearViews();
        static uint64_t getTimestep() {
            return mTimestep;
        }
        static void setTimestep(uint64_t timestep) {
            mTimestep = timestep;
        }
        static void setStreamingMode(StreamingMode mode) {
            mStreamingMode = mode;
        }
        static StreamingMode getStreamingMode() {
            return mStreamingMode;
        }
    public slots:
        void run();
    signals:
        void finished();
    private:

        bool mUpdateThreadIsStopped;
        bool mIsRunning;
        std::condition_variable mUpdateThreadConditionVariable;
        std::mutex mUpdateThreadMutex;

        QThread* mMainThread;

        std::vector<View*> mViews;

        static uint64_t mTimestep;
        static StreamingMode mStreamingMode;
};

}

#endif
