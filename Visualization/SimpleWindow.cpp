#include "SimpleWindow.hpp"
#include <QHBoxLayout>
#include <QApplication>
#include "WindowWidget.hpp"
#include <QEventLoop>

using namespace fast;

void SimpleWindow::addRenderer(Renderer::pointer renderer) {
    mView->addRenderer(renderer);
}

void SimpleWindow::setMaximumFramerate(unsigned int framerate) {
    mView->setMaximumFramerate(framerate);
}

// Make sure only one QApplication is created
void SimpleWindow::initializeQtApp() {
    static bool QtAppIsCreated = false;
    if(!QtAppIsCreated) {
        // Qt Application has not been created, do it now
        std::cout << "creating qt app in SimpleWindow" << std::endl;
        // Create some dummy argc and argv options as QApplication requires it
        int* argc = new int[1];
        *argc = 1;
        const char * argv = "asd";
        QApplication* app = new QApplication(*argc,(char**)&argv);
        QtAppIsCreated = true;
    }
}

QGLContext* SimpleWindow::mGLContext = NULL;

SimpleWindow::SimpleWindow() {
    initializeQtApp();

    mView = View::New();
    // TODO unglobalize mGLContext
    if(mGLContext != NULL){
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
}

void SimpleWindow::runMainLoop() {

    mWidget = new WindowWidget(mView);

    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->addWidget(mView.getPtr().get());
    mWidget->setLayout(mainLayout);
    mWidget->setWindowTitle("FAST");
    mWidget->setContentsMargins(0, 0, 0, 0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mWidget->resize(mWidth,mHeight);
    mView->resize(mWidth,mHeight);

    mWidget->show();
    //QApplication::exec();
    QEventLoop* loop = new QEventLoop;
    if(mTimeout > 0) {
        QTimer* timer = new QTimer(mWidget);
        timer->start(mTimeout);
        timer->setSingleShot(true);
        mWidget->connect(timer,SIGNAL(timeout()),mWidget,SLOT(close()));
        mWidget->connect(timer,SIGNAL(timeout()),loop,SLOT(quit()));
    }

    loop->exec();
}

void SimpleWindow::setWindowSize(unsigned int w, unsigned int h) {
    mWidth = w;
    mHeight = h;
}

void SimpleWindow::setTimeout(unsigned int milliseconds) {
    mTimeout = milliseconds;
}
