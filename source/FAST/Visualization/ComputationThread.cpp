#include "ComputationThread.hpp"
#include "SimpleWindow.hpp"
#include "View.hpp"
#include <QGLContext>
#include <QApplication>

namespace fast {

ComputationThread::ComputationThread() {
    mIsRunning = false;
    mStop = false;
}

ComputationThread::~ComputationThread() {
    if(mIsRunning)
        stop();
    reportInfo() << "Computation thread object destroyed" << Reporter::end();
}

bool ComputationThread::isRunning() {
    std::lock_guard<std::mutex> lock(mUpdateThreadMutex); // this locks the mutex
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
            mViews = getViews();
            m_processObjects = getProcessObjects();
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
    mainGLContext->moveToThread(QApplication::instance()->thread());

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
    if(!mIsRunning)
        return;
    mStop = true;
	auto views = getViews();
	auto m_processObjects = getProcessObjects();

    // This is run in the main thread
    reportInfo() << "Stopping pipelines and waking any blocking threads..." << Reporter::end();
    for(View* view : views) {
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
    for(View* view : views) {
        view->resetRenderers();
    }

    for(auto po : m_processObjects)
        po->stopPipeline();

    // TODO why is this needed? Should already have been done..
    QGLContext* mainGLContext = Window::getMainGLContext();
    if(!mainGLContext->isValid()) {
        throw Exception("QGL context is invalid!");
    }
    mainGLContext->moveToThread(QApplication::instance()->thread());
}

void ComputationThread::start() {
    // Start computation thread using QThreads which is a strange thing, see https://mayaposch.wordpress.com/2011/11/01/how-to-really-truly-use-qthreads-the-full-explanation/
    reportInfo() << "Trying to start computation thread" << Reporter::end();
    QThread* thread = new QThread();
    moveToThread(thread);
    connect(thread, SIGNAL(started()), this, SLOT(run()));
    connect(this, SIGNAL(finished()), thread, SLOT(quit()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

    std::weak_ptr<Window> ptr = std::static_pointer_cast<Window>(mPtr.lock());
    QGLContext* mainGLContext = Window::getMainGLContext();
    if(!mainGLContext->isValid()) {
        throw Exception("QGL context is invalid!");
    }

    mainGLContext->doneCurrent();
    mainGLContext->moveToThread(thread);
    thread->start();
    reportInfo() << "Computation thread started" << Reporter::end();
}

void ComputationThread::addView(View *view) {
    std::lock_guard<std::mutex> lock(mUpdateThreadMutex);
    m_views.push_back(view);
}

void ComputationThread::clearViews() {
    std::lock_guard<std::mutex> lock(mUpdateThreadMutex);
    m_views.clear();
}

View *ComputationThread::getView(int index) const {
    return m_views.at(index);
}

std::vector<View *> ComputationThread::getViews() const {
    return m_views;
}

void ComputationThread::addProcessObject(std::shared_ptr<ProcessObject> po) {
    std::lock_guard<std::mutex> lock(mUpdateThreadMutex);
    m_processObjects.push_back(po);
}

void ComputationThread::clearProcessObjects() {
    std::lock_guard<std::mutex> lock(mUpdateThreadMutex);
    m_processObjects.clear();
}

std::shared_ptr<ProcessObject> ComputationThread::getProcessObjects(int index) const {
    return m_processObjects.at(index);
}

std::vector<std::shared_ptr<ProcessObject>> ComputationThread::getProcessObjects() const {
    return m_processObjects;
}

}
