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
    mView = View::New();

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(mView.getPtr().get());
    setLayout(mainLayout);
    setWindowTitle(tr("FAST"));
    resize(512,512); // default window size
}

void SimpleWindow::runMainLoop() {

    show();
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
    this->resize(w,h);
}
