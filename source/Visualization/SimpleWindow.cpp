#include "SimpleWindow.hpp"
#include <QHBoxLayout>
#include <QApplication>
#include "WindowWidget.hpp"

using namespace fast;

void SimpleWindow::addRenderer(Renderer::pointer renderer) {
    mView->addRenderer(renderer);
}

void SimpleWindow::setMaximumFramerate(unsigned int framerate) {
    mView->setMaximumFramerate(framerate);
}

// Make sure only one QApplication is created
void SimpleWindow::initializeQtApp() {
    if(!QApplication::instance()) {
        // Qt Application has not been created, do it now
        std::cout << "Creating Qt application in SimpleWindow" << std::endl;
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
    }
}

QGLContext* SimpleWindow::mGLContext = NULL;

SimpleWindow::SimpleWindow() {
    initializeQtApp();

	mEventLoop = NULL;
    mView = View::New();
    // TODO unglobalize mGLContext
    if(mGLContext != NULL){
    	// This is used on Mac machines, create a shared context from mGLContext using Qt and give this to the view
        std::cout << "Creating custom QGLContext" << std::endl;
        QGLContext* context2 = new QGLContext(QGLFormat::defaultFormat(), mView.getPtr().get());
        context2->create(mGLContext);
        mView->setContext(context2);
        if(!context2->isValid()) {
            std::cout << "QGL context 2 is invalid!" << std::endl;
            exit(-1);
        }
        if(context2->isSharing()) {
            std::cout << "context 2 is sharing" << std::endl;
        }
    }


    // default window size
    mWidth = 512;
    mHeight = 512;

    mTimeout = 0;

    mWidget = new WindowWidget(mView);
    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->addWidget(mView.getPtr().get());
    mWidget->setLayout(mainLayout);
    mWidget->setWindowTitle("FAST");
    mWidget->setContentsMargins(0, 0, 0, 0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
}

void SimpleWindow::start() {
    mWidget->resize(mWidth,mHeight);
    mView->resize(mWidth,mHeight);

    mWidget->show();
    std::cout << "running main loop" << std::endl;


    mEventLoop = new QEventLoop(mWidget);
    if(mTimeout > 0) {
        QTimer* timer = new QTimer(mWidget);
        timer->start(mTimeout);
        timer->setSingleShot(true);
        mWidget->connect(timer,SIGNAL(timeout()),mWidget,SLOT(close()));
    }

    mWidget->setEventLoop(mEventLoop);
    mEventLoop->exec();
}

SimpleWindow::~SimpleWindow() {
    // Cleanup
	if(mEventLoop != NULL)
		delete mEventLoop;
}

void SimpleWindow::setWindowSize(unsigned int w, unsigned int h) {
    mWidth = w;
    mHeight = h;
}

void SimpleWindow::setTimeout(unsigned int milliseconds) {
    mTimeout = milliseconds;
}

void SimpleWindow::set2DMode() {
    mView->set2DMode();
}

void SimpleWindow::set3DMode() {
    mView->set3DMode();
}

View::pointer SimpleWindow::getView() const {
    return mView;
}
