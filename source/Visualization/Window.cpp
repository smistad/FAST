#include "Window.hpp"
#include <QApplication>

namespace fast {

QGLContext* Window::mMainGLContext = NULL;

Window::Window() {
    mThread = NULL;
    initializeQtApp();
	mEventLoop = NULL;
    mWidget = new WindowWidget;
    mWidget->connect(mWidget, SIGNAL(widgetHasClosed()), this, SLOT(stop()));
}

void Window::initializeQtApp() {
    // Make sure only one QApplication is created
    if(!QApplication::instance()) {
        // Qt Application has not been created, do it now
        std::cout << "Creating Qt application in Window" << std::endl;

        // Create some dummy argc and argv options as QApplication requires it
        int* argc = new int[1];
        *argc = 0;
        QApplication* app = new QApplication(*argc,NULL);

        // There is a bug in AMD OpenCL related to comma (,) as decimal point
        // This will change decimal point to dot (.)
        struct lconv * lc;
        lc = localeconv();
        if(strcmp(lc->decimal_point, ",") == 0) {
            std::cout << "WARNING: Your system uses comma as decimal point." << std::endl;
            std::cout << "WARNING: This will now be changed to dot to avoid any comma related bugs." << std::endl;
            setlocale(LC_NUMERIC, "C");
            // Check again to be sure
            lc = localeconv();
            if(strcmp(lc->decimal_point, ",") == 0) {
                throw Exception("Failed to convert decimal point to dot.");
            }
        }
        // Dummy widget
        QGLWidget* widget = new QGLWidget;

        // Create GL context to be shared with the CL contexts
        mMainGLContext = new QGLContext(QGLFormat::defaultFormat(), widget); // by including widget here the context becomes valid
        mMainGLContext->create();
        if(!mMainGLContext->isValid()) {
            throw Exception("Qt GL context is invalid!");
        }
    }
}

void Window::stop() {
    std::cout << "Stop signal recieved.." << std::endl;
    stopComputationThread();
    if(mEventLoop != NULL)
        mEventLoop->quit();
    /*
    // Make sure event is not quit twice
    if(!mWidget->getView()->hasQuit()) {
        // This event occurs when window is closed or timeout is reached
        mWidget->getView()->quit();
    }
    */
}

void Window::start() {
    mWidget->resize(mWidth,mHeight);
    mWidget->getView()->resize(mWidth,mHeight);

    mWidget->show();
    std::cout << "running main loop" << std::endl;

    mEventLoop = new QEventLoop(mWidget);
    if(mTimeout > 0) {
        QTimer* timer = new QTimer(mWidget);
        timer->start(mTimeout);
        timer->setSingleShot(true);
        mWidget->connect(timer,SIGNAL(timeout()),mWidget,SLOT(close()));
    }

    startComputationThread();

    mEventLoop->exec(); // This call blocks and starts rendering
}

Window::~Window() {
    std::cout << "Destroying window.." << std::endl;
    // Cleanup
    std::cout << "Deleting event loop" << std::endl;
    if(mEventLoop != NULL)
        delete mEventLoop;
    std::cout << "Deleting widget" << std::endl;
    if(mWidget != NULL)
        delete mWidget;
    std::cout << "Deleting computation thread" << std::endl;
    if(mThread != NULL) {
        mThread->stop();
        delete mThread;
    }
    std::cout << "Window destroyed" << std::endl;
}

void Window::setTimeout(unsigned int milliseconds) {
    mTimeout = milliseconds;
}

QGLContext* Window::getMainGLContext() {
    if(mMainGLContext == NULL) {
        initializeQtApp();
    }

    return mMainGLContext;
}

void Window::startComputationThread() {
    if(mThread == NULL) {
        // Start computation thread
        std::cout << "Trying to start computation thread" << std::endl;
        mThread = new ComputationThread(QThread::currentThread());
        mThread->addView(mWidget->getView());
        QGLContext* mainGLContext = Window::getMainGLContext();
        if(!mainGLContext->isValid()) {
            throw Exception("QGL context is invalid!");
        }

        // Context must be current in this thread before it can be moved to another thread
        //doneCurrent();
        mainGLContext->makeCurrent();
        mainGLContext->moveToThread(mThread);
        mainGLContext->doneCurrent();
        mThread->start();
        std::cout << "Computation thread started" << std::endl;
        //makeCurrent();
    }
}

void Window::stopComputationThread() {
    std::cout << "Trying to stop computation thread" << std::endl;
    mThread->stop();
    delete mThread;
    mThread = NULL;
    std::cout << "Computation thread stopped" << std::endl;
}

} // end namespace fast
