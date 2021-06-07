#include "SimpleWindow.hpp"
#include <QHBoxLayout>

namespace fast {

void SimpleWindow::addRenderer(std::shared_ptr<Renderer> renderer) {
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

SimpleWindow::SimpleWindow(bool mode2D, Color bgcolor, uint width, uint height) {
    init();
    if(mode2D) {
        set2DMode();
    } else {
        set3DMode();
    }
    getView()->setBackgroundColor(std::move(bgcolor));
    if(width > 0)
        setWidth(width);
    if(height > 0)
        setHeight(height);
}

void SimpleWindow::init() {

    View* view = createView();

    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->addWidget(view);
    mWidget->setLayout(mainLayout);
    mWidget->setContentsMargins(0, 0, 0, 0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
}

View* SimpleWindow::getView() {
    return Window::getView(0);
}

} // end namespace fast
