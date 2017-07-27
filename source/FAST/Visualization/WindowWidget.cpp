#include "WindowWidget.hpp"
#include "SimpleWindow.hpp"
using namespace fast;


void WindowWidget::keyPressEvent(QKeyEvent* event) {
    // Close window if user presses Q or escape
    switch(event->key()) {
    case Qt::Key_Q:
    case Qt::Key_Escape:
        Reporter::info() << "Close key pressed" << Reporter::end();
        close();
        return;
        break;
    }
    for(View* view : mViews)
        view->keyPressEvent(event);
}

void WindowWidget::closeEvent(QCloseEvent* event) {
    Reporter::info() << "Close event triggered" << Reporter::end();
    emit widgetHasClosed();
}

WindowWidget::~WindowWidget() {
    Reporter::info() << "DESTROYING window widget and all related views" << Reporter::end();
    for(View* view : mViews) {
        delete view;
    }
}

WindowWidget::WindowWidget() {
    setWindowTitle("FAST");
}

View* WindowWidget::addView() {
    View* view = new View();
    mViews.push_back(view);
    return view;
}

void WindowWidget::clearViews() {
    mViews.clear();
}

std::vector<View*> WindowWidget::getViews() const {
    return mViews;
}


