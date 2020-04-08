#include "ComputationThread.hpp"
#include "SimpleWindow.hpp"
#include "View.hpp"
#include <QGLContext>

namespace fast {

ComputationThread::ComputationThread(QThread* mainThread) {
    mIsRunning = false;
    mStop = false;
    mMainThread = mainThread;
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

    uint executeToken = 0;
    while(true) {
        {
            std::unique_lock<std::mutex> lock(mUpdateThreadMutex); // this locks the mutex
            if(mStop)
                break;
        }
        try {
            for(auto po : m_processObjects)
                po->update(executeToken);
            for(View *view : mViews) {
                view->updateRenderersInput(executeToken);
            }
            for(View *view : mViews) {
                view->updateRenderers();
            }
        } catch(ThreadStopped &e) {
            break;
        }
        ++executeToken;
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

void ComputationThread::setProcessObjects(std::vector<SharedPointer<ProcessObject>> pos) {
    m_processObjects = pos;
}

}
