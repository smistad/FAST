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
MultiViewWindow::MultiViewWindow(int viewCount, Color bgcolor, int width, int height, bool verticalMode) {
    clearViews();
    for(int i = 0; i < viewCount; i++) {
        auto view = createView();
    }
    setBackgroundColor(bgcolor);
    if(width > 0)
        setWidth(width);
    if(height > 0)
        setHeight(height);
    if(verticalMode)
        setVerticalMode();
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
    mThread->addView(view);
}

void MultiViewWindow::start() {
    createLayout();
    Window::start();
}

std::shared_ptr<MultiViewWindow> MultiViewWindow::connect(int viewNr, std::shared_ptr<Renderer> renderer) {
    addRenderer(viewNr, renderer);
    return std::dynamic_pointer_cast<MultiViewWindow>(mPtr.lock());
}

std::shared_ptr<MultiViewWindow>
MultiViewWindow::connect(int viewNr, std::vector<std::shared_ptr<Renderer>> renderers) {
    for(auto renderer : renderers)
        addRenderer(viewNr, renderer);
    return std::dynamic_pointer_cast<MultiViewWindow>(mPtr.lock());
}

void MultiViewWindow::setBackgroundColor(Color color) {
    for(auto view : getViews())
        view->setBackgroundColor(color);
}

} // end namespace fast
