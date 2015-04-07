#include "SimpleWindow.hpp"
#include <QHBoxLayout>

namespace fast {

void SimpleWindow::addRenderer(Renderer::pointer renderer) {
    mWidget->getView()->addRenderer(renderer);
}

void SimpleWindow::removeAllRenderers() {
    mWidget->getView()->removeAllRenderers();
}

void SimpleWindow::setMaximumFramerate(unsigned int framerate) {
    mWidget->getView()->setMaximumFramerate(framerate);
}

SimpleWindow::~SimpleWindow() {

}

SimpleWindow::SimpleWindow() {
    std::cout << "Creating custom Qt GL context for the view which shares with the primary GL context" << std::endl;
    QGLContext* context = new QGLContext(QGLFormat::defaultFormat(), mWidget->getView().getPtr().get());
    context->create(getMainGLContext());
    mWidget->getView()->setContext(context);
    if(!context->isValid()) {
        std::cout << "The custom Qt GL context is invalid!" << std::endl;
        exit(-1);
    }
    if(context->isSharing()) {
        std::cout << "The custom Qt GL context is sharing" << std::endl;
    }

    // default window size
    mWidth = 512;
    mHeight = 512;

    mTimeout = 0;

    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->addWidget(mWidget->getView().getPtr().get());
    mWidget->setLayout(mainLayout);
    mWidget->setWindowTitle("FAST");
    mWidget->setContentsMargins(0, 0, 0, 0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
}

void SimpleWindow::setWindowSize(unsigned int w, unsigned int h) {
    mWidth = w;
    mHeight = h;
}

void SimpleWindow::set2DMode() {
    mWidget->getView()->set2DMode();
}

void SimpleWindow::set3DMode() {
    mWidget->getView()->set3DMode();
}

View::pointer SimpleWindow::getView() const {
    return mWidget->getView();
}

} // end namespace fast
