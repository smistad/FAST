#include "DualViewWindow.hpp"
#include <QHBoxLayout>

namespace fast {

void DualViewWindow::addRendererToTopLeftView(
        Renderer::pointer renderer) {
    getViews()[0]->addRenderer(renderer);
}

void DualViewWindow::addRendererToBottomRightView(
        Renderer::pointer renderer) {
    getViews()[1]->addRenderer(renderer);
}

void DualViewWindow::removeAllRenderers() {
    getViews()[0]->removeAllRenderers();
    getViews()[1]->removeAllRenderers();
}

View* DualViewWindow::getTopLeftView() const {
    return getViews()[0];
}

View* DualViewWindow::getBottomRightView() const {
    return getViews()[1];
}

void DualViewWindow::setHorizontalMode() {
}

void DualViewWindow::setVerticalMode() {
}

DualViewWindow::~DualViewWindow() {
}

DualViewWindow::DualViewWindow() {
    View* view1 = createView();
    View* view2 = createView();

    // default window size
    mWidth = 512;
    mHeight = 512;

    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->addWidget(view1);
    mainLayout->addWidget(view2);
    mWidget->setLayout(mainLayout);
    mWidget->setWindowTitle("FAST");
    mWidget->setContentsMargins(0, 0, 0, 0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
}

} // end namespace fast
