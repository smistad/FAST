#include "Window.hpp"
#include <QApplication>

namespace fast {

QGLContext* Window::mMainGLContext = NULL;

Window::Window() {
    mThread = NULL;
    mTimeout = 0;
    initializeQtApp();
	mEventLoop = NULL;
    mWidget = new WindowWidget;
    mEventLoop = new QEventLoop(mWidget);
    mWidget->connect(mWidget, SIGNAL(widgetHasClosed()), this, SLOT(stop()));
    mWidget->setWindowTitle("FAST");
    mWidget->setContentsMargins(0, 0, 0, 0);

    // default window size
    mWidth = 512;
    mHeight = 512;
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
    }

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

    // Create computation GL context, if it doesn't exist
    if(mMainGLContext == NULL) {
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

View* Window::createView() {
    View* view = new View();
    mWidget->addView(view);

    return view;
}

void Window::start() {
    mWidget->resize(mWidth,mHeight);
    //getViews()[0]->resize(mWidth,mHeight);

    mWidget->show();
    std::cout << "running main loop" << std::endl;

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
    // Cleanup
    std::cout << "Destroying window.." << std::endl;
    // Event loop is child of widget
    //std::cout << "Deleting event loop" << std::endl;
    //if(mEventLoop != NULL)
    //    delete mEventLoop;
    std::cout << "Deleting widget" << std::endl;
    if(mWidget != NULL)
        delete mWidget;
    std::cout << "Finished deleting window widget" << std::endl;
    if(mThread != NULL) {
        mThread->stop();
        delete mThread;
        mThread = NULL;
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
        // Start computation thread using QThreads which is a strange thing, see https://mayaposch.wordpress.com/2011/11/01/how-to-really-truly-use-qthreads-the-full-explanation/
        std::cout << "Trying to start computation thread" << std::endl;
        mThread = new ComputationThread(QThread::currentThread());
        QThread* thread = new QThread();
        mThread->moveToThread(thread);
        connect(thread, SIGNAL(started()), mThread, SLOT(run()));
        connect(mThread, SIGNAL(finished()), thread, SLOT(quit()));
        //connect(mThread, SIGNAL(finished()), mThread, SLOT(deleteLater()));
        connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

        // Make sure this thread is deleted after it is finished
        //connect(mThread, SIGNAL(finished()), mThread, SLOT(deleteLater()));

        for(int i = 0; i < getViews().size(); i++)
            mThread->addView(getViews()[i]);
        QGLContext* mainGLContext = Window::getMainGLContext();
        if(!mainGLContext->isValid()) {
            throw Exception("QGL context is invalid!");
        }

        // Context must be current in this thread before it can be moved to another thread
        mainGLContext->makeCurrent();
        mainGLContext->moveToThread(thread);
        mainGLContext->doneCurrent();
        thread->start();
        std::cout << "Computation thread started" << std::endl;
    }
}

void Window::stopComputationThread() {
    std::cout << "Trying to stop computation thread" << std::endl;
    if(mThread != NULL) {
        mThread->stop();
        delete mThread;
        mThread = NULL;
    }
    std::cout << "Computation thread stopped" << std::endl;
}

std::vector<View*> Window::getViews() const {
    return mWidget->getViews();
}

View* Window::getView(uint i) const {
    return mWidget->getViews().at(i);
}

void Window::setWidth(uint width) {
    mWidth = width;
}

void Window::setHeight(uint height) {
    mHeight = height;
}

} // end namespace fast
