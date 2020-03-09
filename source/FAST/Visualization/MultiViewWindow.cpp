#include "MultiViewWindow.hpp"
#include <QHBoxLayout>
#include <QVBoxLayout>

namespace fast {

void MultiViewWindow::addRenderer(
        int viewIndex,
        Renderer::pointer renderer) {
    if(getViews().size() == 0)
        throw Exception("You must set the nr of views before adding renderers to MultiViewWindow");
    getView(viewIndex)->addRenderer(renderer);
}

void MultiViewWindow::removeAllRenderers() {
    std::vector<View*> views = getViews();
    for(View* view : views) {
        view->removeAllRenderers();
    }
}

void MultiViewWindow::setNrOfViews(int viewCount) {
    mWidget->clearViews();
    for(int i = 0; i < viewCount; i++) {
        createView();
    }
}

void MultiViewWindow::setHorizontalMode() {
    mVerticalMode = false;
    createLayout();
}

void MultiViewWindow::setVerticalMode() {
    mVerticalMode = true;
    createLayout();
}

MultiViewWindow::~MultiViewWindow() {
}

MultiViewWindow::MultiViewWindow() {
    mVerticalMode = false;
}

void MultiViewWindow::createLayout() {
    // Remove old layout by deleting it
    QLayout* layout = mWidget->layout();
    delete layout;

    // Add new layout
    std::vector<View*> views = getViews();
    if(mVerticalMode) {
        QVBoxLayout* mainLayout = new QVBoxLayout;
        for(View* view : views) {
            mainLayout->addWidget(view);
        }
        mainLayout->setContentsMargins(0, 0, 0, 0);
        mWidget->setLayout(mainLayout);
    } else {
        QHBoxLayout* mainLayout = new QHBoxLayout;
        for(View* view : views) {
            mainLayout->addWidget(view);
        }
        mainLayout->setContentsMargins(0, 0, 0, 0);
        mWidget->setLayout(mainLayout);
    }
}

void MultiViewWindow::addView(View* view) {
    mWidget->addView(view);
}

void MultiViewWindow::start() {
    createLayout();
    Window::start();
}

} // end namespace fast
