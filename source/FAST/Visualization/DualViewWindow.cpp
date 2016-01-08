#include "DualViewWindow.hpp"
#include <QHBoxLayout>
#include <QVBoxLayout>

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
        mainLayout->addWidget(getViews()[0]);
        mainLayout->addWidget(getViews()[1]);
        mainLayout->setContentsMargins(0, 0, 0, 0);
        mWidget->setLayout(mainLayout);
    } else {
        QHBoxLayout* mainLayout = new QHBoxLayout;
        mainLayout->addWidget(getViews()[0]);
        mainLayout->addWidget(getViews()[1]);
        mainLayout->setContentsMargins(0, 0, 0, 0);
        mWidget->setLayout(mainLayout);
    }
}

} // end namespace fast
