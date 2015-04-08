#include "SimpleWindow.hpp"

namespace fast {

void SimpleWindow::addRenderer(Renderer::pointer renderer) {
    getViews()[0]->addRenderer(renderer);
}

void SimpleWindow::removeAllRenderers() {
    getViews()[0]->removeAllRenderers();
}

void SimpleWindow::setMaximumFramerate(unsigned int framerate) {
    getViews()[0]->setMaximumFramerate(framerate);
}

SimpleWindow::~SimpleWindow() {
}

SimpleWindow::SimpleWindow() {
    View* view = createView();

    // default window size
    mWidth = 512;
    mHeight = 512;

    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->addWidget(view);
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
    getViews()[0]->set2DMode();
}

void SimpleWindow::set3DMode() {
    getViews()[0]->set3DMode();
}

View::pointer SimpleWindow::getView() const {
    return (getViews()[0]);
}

} // end namespace fast
