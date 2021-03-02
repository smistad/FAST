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
		bool canUpdate = false;
        std::vector<View*> mViews;
        std::vector<std::shared_ptr<ProcessObject>> m_processObjects;
        {
            std::unique_lock<std::mutex> lock(mUpdateThreadMutex); // this locks the mutex
            if(m_window.expired())
                break;
            auto window = m_window.lock();
            mViews = window->getViews();
            m_processObjects = window->getProcessObjects();
            if(mStop)
                break;
            if(m_processObjects.size() > 0)
                canUpdate = true;
            for(View* view : mViews) {
                auto rendererList = view->getRenderers();
                if(rendererList.size() > 0)
                    canUpdate = true;
            }
        }
		if(!canUpdate) { // There is nothing for this computation thread to do atm, sleep a short while
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
			continue;
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
            reportInfo() << "Thread stopped exception occured in ComputationThread, exiting.." << reportEnd();
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
	if(m_window.expired())
		return;
	auto window = m_window.lock();
	auto mViews = window->getViews();
	auto m_processObjects = window->getProcessObjects();

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

    for(auto po : m_processObjects)
        po->stopPipeline();
}

void ComputationThread::setWindow(std::weak_ptr<Window> ptr) {
    m_window = ptr;
}

}
