#include "WindowWidget.hpp"
#include "SimpleWindow.hpp"
using namespace fast;


void WindowWidget::keyPressEvent(QKeyEvent* event) {
    // Close window if user presses Q or escape
    switch(event->key()) {
    case Qt::Key_Q:
    case Qt::Key_Escape:
        close();
        return;
        break;
    }
    for(View* view : mViews)
        view->keyPressEvent(event);
}

void WindowWidget::closeEvent(QCloseEvent* event) {
    emit widgetHasClosed();
}

WindowWidget::~WindowWidget() {
    //std::cout << "DESTROYING window widget" << std::endl;
}

WindowWidget::WindowWidget() {
}

void WindowWidget::addView(View* view) {
    mViews.push_back(view);
}

std::vector<View*> WindowWidget::getViews() const {
    return mViews;
}


