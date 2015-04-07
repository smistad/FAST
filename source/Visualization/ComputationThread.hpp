#ifndef COMPUTATION_THREAD_HPP
#define COMPUTATION_THREAD_HPP

#include "View.hpp"
#include <QThread>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <vector>


namespace fast {

class ComputationThread : public QThread {
    public:
        ComputationThread(QThread* mainThread);
        bool isRunning();
        void stop();
        void addView(View::pointer view);
        void clearViews();
    private:
        void run();

        bool mUpdateThreadIsStopped;
        bool mIsRunning;
        boost::condition_variable mUpdateThreadConditionVariable;
        boost::mutex mUpdateThreadMutex;

        QThread* mMainThread;

        std::vector<View::pointer> mViews;
};

}

#endif
