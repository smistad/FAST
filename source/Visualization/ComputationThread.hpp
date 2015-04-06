#ifndef COMPUTATION_THREAD_HPP
#define COMPUTATION_THREAD_HPP

#include "View.hpp"
#include <QThread>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>


namespace fast {

class ComputationThread : public QThread {
    public:
        ComputationThread();
        View::pointer mView;
        QThread* mMainThread;
        bool isRunning();
        void stop();
    private:
        void run();

        bool mUpdateThreadIsStopped;
        bool mIsRunning;
        boost::condition_variable mUpdateThreadConditionVariable;
        boost::mutex mUpdateThreadMutex;
};

}

#endif
