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
        ComputationThread(QThread* mainThread, StreamingMode mode);
        ~ComputationThread();
        bool isRunning();
        void stop();
        void addView(View* view);
        void clearViews();
        uint64_t getTimestep();
        void setTimestep(uint64_t timestep);
        StreamingMode getStreamingMode();
        void setStreamingMode(StreamingMode mode);
        /**
         * This will stop the timestep from incrementing
         */
        void pause();
        /**
         * Resume after pausing
         */
        void resume();
        /**
         * Set timestep limit
         */
        void setTimestepLimit(uint64_t timestep);
        /**
         * Enable/disable looping of timestep, only works if timestep limit is set
         * @param loop
         */
        void setTimestepLoop(bool loop);

    public slots:
        void run();
    signals:
        void finished();
        void timestepIncreased();
    private:

        bool mIsRunning;
        std::condition_variable mUpdateThreadConditionVariable;
        std::mutex mUpdateThreadMutex;

        QThread* mMainThread;

        std::vector<View*> mViews;

        uint64_t mTimestep;
        uint64_t mTimestepLimit = 0;
        bool mPaused = false;
        bool mLoop = false;
        bool mStop = false;
        StreamingMode mStreamingMode;
};

}

#endif
