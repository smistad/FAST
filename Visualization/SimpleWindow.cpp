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

SimpleWindow::SimpleWindow() {
    // Create some dummy argc and argv options as QApplication requires it
    int* argc = new int[1];
    *argc = 1;
    const char * argv = "asd";
    QApplication* app = new QApplication(*argc,(char**)&argv);
    mView = View::New();

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

    if(mTimeout > 0) {
        QTimer* timer = new QTimer(mWidget);
        timer->start(mTimeout);
        timer->setSingleShot(true);
        mWidget->connect(timer,SIGNAL(timeout()),mWidget,SLOT(close()));
    }

    mWidget->show();
    QApplication::exec();
}

void SimpleWindow::setWindowSize(unsigned int w, unsigned int h) {
    mWidth = w;
    mHeight = h;
}

void SimpleWindow::setTimeout(unsigned int milliseconds) {
    mTimeout = milliseconds;
}
