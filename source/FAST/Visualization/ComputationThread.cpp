#include "ComputationThread.hpp"
#include "SimpleWindow.hpp"
#include "View.hpp"
#include <QGLContext>

namespace fast {

ComputationThread::ComputationThread(QThread* mainThread, StreamingMode mode) {
    mIsRunning = false;
    mStop = false;
    mMainThread = mainThread;
    mTimestep = 0;
    mStreamingMode = mode;
}

void ComputationThread::addView(View* view) {
    mViews.push_back(view);
}

void ComputationThread::clearViews() {
    mViews.clear();
}

ComputationThread::~ComputationThread() {
    reportInfo() << "Computation thread object destroyed" << Reporter::end();
}

bool ComputationThread::isRunning() {
    return mIsRunning;
}

void ComputationThread::run() {
    // This is run in the secondary (computation thread)
    {
        std::unique_lock<std::mutex> lock(mUpdateThreadMutex); // this locks the mutex
        mIsRunning = true;
        mStop = false;
    }
    QGLContext* mainGLContext = Window::getMainGLContext();
    mainGLContext->makeCurrent();
    mTimestep = 0;

    while(true) {
        if(!mPaused) {
            mTimestep++;
            if(mLoop && mTimestep == mTimestepLimit)
                mTimestep = 0;
            emit timestepIncreased();
        }
        {
            std::unique_lock<std::mutex> lock(mUpdateThreadMutex); // this locks the mutex
            if(mStop)
                break;
        }
        try {
            for (View *view : mViews) {
                view->updateRenderersInput(mTimestep, mStreamingMode);
            }
            for (View *view : mViews) {
                view->updateRenderers(mTimestep, mStreamingMode);
            }
        } catch(ThreadStopped &e) {
            break;
        }
    }

    // Move GL context back to main thread
    mainGLContext->doneCurrent();
    mainGLContext->moveToThread(mMainThread);

    emit finished();
    reportInfo() << "Computation thread has finished in run()" << Reporter::end();
    {
        std::unique_lock<std::mutex> lock(mUpdateThreadMutex); // this locks the mutex
        mIsRunning = false;
    }
    mUpdateThreadConditionVariable.notify_one();
}

void ComputationThread::stop() {
    std::unique_lock<std::mutex> lock(mUpdateThreadMutex); // this locks the mutex
    mStop = true;
    // This is run in the main thread
    reportInfo() << "Stopping pipelines and waking any blocking threads..." << Reporter::end();
    for(View* view : mViews) {
        view->stopRenderers();
    }
    reportInfo() << "Pipelines stopped" << Reporter::end();
    reportInfo() << "Stopping computation thread..." << Reporter::end();
    // Block until mIsRunning is set to false
    while(mIsRunning) {
        // Unlocks the mutex and wait until someone calls notify.
        // When it wakes, the mutex is locked again and mIsRunning is checked.
        mUpdateThreadConditionVariable.wait(lock);
    }
    reportInfo() << "Computation thread stopped" << Reporter::end();
    for(View* view : mViews) {
        view->resetRenderers();
    }
}


uint64_t ComputationThread::getTimestep() {
    return mTimestep;
}

void ComputationThread::setTimestep(uint64_t timestep) {
    reportInfo() << "Timestep set to " << timestep << reportEnd();
    mTimestep = timestep;
}

StreamingMode ComputationThread::getStreamingMode() {
    return mStreamingMode;
}

void ComputationThread::setStreamingMode(StreamingMode mode) {
    mStreamingMode = mode;
}

void ComputationThread::setTimestepLimit(uint64_t timestep) {
    mTimestepLimit = timestep;
}

void ComputationThread::setTimestepLoop(bool loop) {
    mLoop = loop;
}

void ComputationThread::pause() {
    mPaused = true;
}

void ComputationThread::resume() {
    mPaused = false;
}

}
