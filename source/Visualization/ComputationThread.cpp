#include "ComputationThread.hpp"
#include "SimpleWindow.hpp"

namespace fast {

ComputationThread::ComputationThread(QThread* mainThread) {
    mUpdateThreadIsStopped = false;
    mIsRunning = false;
    mMainThread = mainThread;
}

void ComputationThread::addView(View* view) {
    mViews.push_back(view);
}

void ComputationThread::clearViews() {
    mViews.clear();
}

ComputationThread::~ComputationThread() {
    std::cout << "Computation thread object destroyed" << std::endl;
}

bool ComputationThread::isRunning() {
    return mIsRunning;
}

void ComputationThread::run() {
    // This is run in the secondary (computation thread)
    {
        boost::unique_lock<boost::mutex> lock(mUpdateThreadMutex); // this locks the mutex
        mIsRunning = true;
    }
    QGLContext* mainGLContext = Window::getMainGLContext();
    mainGLContext->makeCurrent();

    while(true) {
        for(int i = 0; i < mViews.size(); i++) {
            mViews[i]->updateAllRenderers();
        }
        boost::unique_lock<boost::mutex> lock(mUpdateThreadMutex); // this locks the mutex
        if(mUpdateThreadIsStopped) {
            // Move GL context back to main thread
            mainGLContext->moveToThread(mMainThread);
            mainGLContext->doneCurrent();
            mIsRunning = false;
            break;
        }
    }

    emit finished();
    std::cout << "Computation thread has finished in run()" << std::endl;
    mUpdateThreadConditionVariable.notify_one();
}

void ComputationThread::stop() {
    // This is run in the main thread
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
