#include "ComputationThread.hpp"
#include "SimpleWindow.hpp"

namespace fast {

ComputationThread::ComputationThread(QThread* mainThread) {
    mUpdateThreadIsStopped = false;
    mIsRunning = false;
    mMainThread = mainThread;
}

void ComputationThread::addView(View::pointer view) {
    mViews.push_back(view);
}

void ComputationThread::clearViews() {
    mViews.clear();
}

bool ComputationThread::isRunning() {
    return mIsRunning;
}

void ComputationThread::run() {
    {
        boost::unique_lock<boost::mutex> lock(mUpdateThreadMutex); // this locks the mutex
        mIsRunning = true;
    }
    SimpleWindow::mGLContext->makeCurrent();

    while(true) {
        for(int i = 0; i < mViews.size(); i++) {
            mViews[i]->updateAllRenderers();
        }
        boost::unique_lock<boost::mutex> lock(mUpdateThreadMutex); // this locks the mutex
        if(mUpdateThreadIsStopped) {
            mIsRunning = false;
            break;
        }
    }
    mUpdateThreadConditionVariable.notify_one();

    // Move GL context back to main thread
    SimpleWindow::mGLContext->moveToThread(mMainThread);
    SimpleWindow::mGLContext->doneCurrent();
    mIsRunning = false;
}

void ComputationThread::stop() {
    boost::unique_lock<boost::mutex> lock(mUpdateThreadMutex); // this locks the mutex
    mUpdateThreadIsStopped = true;
    // Block until mIsRunning is set to false
    while(mIsRunning) {
        // Unlocks the mutex and wait until someone calls notify.
        // When it wakes, the mutex is locked again and mUpdateIsRunning is checked.
        mUpdateThreadConditionVariable.wait(lock);
    }
}


}
