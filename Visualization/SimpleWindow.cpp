#include "SimpleWindow.hpp"
#include <QHBoxLayout>
#include <QApplication>
using namespace fast;

void SimpleWindow::addRenderer(Renderer::pointer renderer) {
    mView->addRenderer(renderer);
}

void SimpleWindow::setMaximumFramerate(unsigned char framerate) {
    mFramerate = framerate;
}

SimpleWindow::SimpleWindow() {
    mFramerate = 25;
    mView = View::New();

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(mView.getPtr().get());
    setLayout(mainLayout);
    setWindowTitle(tr("FAST"));
}

void SimpleWindow::runMainLoop() {

    show();
    QApplication::exec();
}
