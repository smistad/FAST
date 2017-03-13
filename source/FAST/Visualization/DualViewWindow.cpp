#include "DualViewWindow.hpp"
#include <QHBoxLayout>
#include <QVBoxLayout>

namespace fast {

void DualViewWindow::addRendererToTopLeftView(
        Renderer::pointer renderer) {
    getView(0)->addRenderer(renderer);
}

void DualViewWindow::addRendererToBottomRightView(
        Renderer::pointer renderer) {
    getView(1)->addRenderer(renderer);
}

void DualViewWindow::removeAllRenderers() {
    getView(0)->removeAllRenderers();
    getView(1)->removeAllRenderers();
}

View* DualViewWindow::getTopLeftView() const {
    return getView(0);
}

View* DualViewWindow::getBottomRightView() const {
    return getView(1);
}

void DualViewWindow::setHorizontalMode() {
    mVerticalMode = false;
    createLayout();
}

void DualViewWindow::setVerticalMode() {
    mVerticalMode = true;
    createLayout();
}

DualViewWindow::~DualViewWindow() {
}

DualViewWindow::DualViewWindow() {
    mVerticalMode = false;
    View* view1 = createView();
    View* view2 = createView();

    createLayout();
}

void DualViewWindow::createLayout() {
    // Remove old layout by deleting it
    QLayout* layout = mWidget->layout();
    delete layout;

    // Add new layout
    if(mVerticalMode) {
        QVBoxLayout* mainLayout = new QVBoxLayout;
        mainLayout->addWidget(getView(0));
        mainLayout->addWidget(getView(1));
        mWidget->setLayout(mainLayout);
        mainLayout->setContentsMargins(0, 0, 0, 0);
    } else {
        QHBoxLayout* mainLayout = new QHBoxLayout;
        mainLayout->addWidget(getView(0));
        mainLayout->addWidget(getView(1));
        mWidget->setLayout(mainLayout);
        mainLayout->setContentsMargins(0, 0, 0, 0);
    }
}

} // end namespace fast
