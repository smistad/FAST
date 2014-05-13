#include "SimpleWindow.hpp"
#include <QHBoxLayout>
#include <QApplication>
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
    char * argv = "asd";
    QApplication* app = new QApplication(*argc,&argv);

    mWidget = new QWidget;
    mView = View::New();

    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->addWidget(mView.getPtr().get());
    mWidget->setLayout(mainLayout);
    mWidget->setWindowTitle("FAST");
    mWidget->setContentsMargins(0, 0, 0, 0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    setWindowSize(512,512); // default window size
}

void SimpleWindow::runMainLoop() {
    mWidget->show();
    QApplication::exec();
}

void SimpleWindow::keyPressEvent(QKeyEvent* event) {
    mView->keyPressEvent(event);
}

void SimpleWindow::mouseMoveEvent(QMouseEvent* event) {
    mView->mouseMoveEvent(event);
}

void SimpleWindow::mousePressEvent(QMouseEvent* event) {
    mView->mousePressEvent(event);
}

void SimpleWindow::mouseReleaseEvent(QMouseEvent* event) {
    mView->mouseReleaseEvent(event);
}

void SimpleWindow::setWindowSize(unsigned int w, unsigned int h) {
    mWidget->resize(w,h);
    mView->resize(w,h);
}
