#include "WindowWidget.hpp"
#include "SimpleWindow.hpp"
using namespace fast;


void WindowWidget::keyPressEvent(QKeyEvent* event) {
    // Close window if user presses Q or escape
    switch(event->key()) {
    case Qt::Key_Q:
    case Qt::Key_Escape:
        close();
        return;
        break;
    }
    mView->keyPressEvent(event);
}

void WindowWidget::closeEvent(QCloseEvent* event) {
    stopComputationThread();
    // Make sure event is not quit twice
    if(!mView->hasQuit()) {
        // This event occurs when window is closed or timeout is reached
        mView->quit();
        mEventLoop->quit();
    }
}

void WindowWidget::mouseMoveEvent(QMouseEvent* event) {
    mView->mouseMoveEvent(event);
}

void WindowWidget::mousePressEvent(QMouseEvent* event) {
    mView->mousePressEvent(event);
}

void WindowWidget::mouseReleaseEvent(QMouseEvent* event) {
    mView->mouseReleaseEvent(event);
}

void WindowWidget::wheelEvent(QWheelEvent* event) {
    mView->wheelEvent(event);
}

WindowWidget::~WindowWidget() {
    //std::cout << "DESTROYING window widget" << std::endl;
    if(mThread != NULL) {
        mThread->exit();
        delete mThread;
    }
}

View::pointer WindowWidget::getView() {
    return mView;
}

WindowWidget::WindowWidget() {
    mThread = NULL;
    mView = View::New();
    mEventLoop = NULL;
}

void WindowWidget::startComputationThread() {
    // Start computation thread
    std::cout << "Trying to start computation thread" << std::endl;
    delete mThread;
    mThread = new ComputationThread;
    mThread->mView = mView;
    mThread->mMainThread = QThread::currentThread();
    if(!SimpleWindow::mGLContext->isValid()) {
        throw Exception("QGL context is invalid!");
    }

    // Context must be current in this thread before it can be moved to another thread
    //doneCurrent();
    SimpleWindow::mGLContext->makeCurrent();
    SimpleWindow::mGLContext->moveToThread(mThread);
    SimpleWindow::mGLContext->doneCurrent();
    mThread->start();
    std::cout << "Computation thread started" << std::endl;
    //makeCurrent();
}

void WindowWidget::stopComputationThread() {
    std::cout << "Trying to stop computation thread" << std::endl;
    mThread->stop();
    std::cout << "Computation thread stopped" << std::endl;
}
