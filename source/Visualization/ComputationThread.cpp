#include "ComputationThread.hpp"
#include "SimpleWindow.hpp"

namespace fast {

ComputationThread::ComputationThread() {
    mUpdateThreadIsStopped = false;
    mIsRunning = false;
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
    try {
        while(true) {
            mView->updateAllRenderers();
            boost::unique_lock<boost::mutex> lock(mUpdateThreadMutex); // this locks the mutex
            if(mUpdateThreadIsStopped) {
                mIsRunning = false;
                break;
            }
        }
        mUpdateThreadConditionVariable.notify_one();
    } catch(Exception &e) {
        // If window has been killed, pipeline is stopped and should ignore any exceptions
        if(mView.isValid()) {
            if(!mView->hasQuit()) {
                throw e;
            }
        }
    }
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
