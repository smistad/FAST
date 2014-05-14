#include "WindowWidget.hpp"
using namespace fast;


void WindowWidget::keyPressEvent(QKeyEvent* event) {
    // Close window if user presses Q or escape
    switch(event->key()) {
    case Qt::Key_Q:
    case Qt::Key_Escape:
        close();
        break;
    }
    mView->keyPressEvent(event);
}

void WindowWidget::mouseMoveEvent(QMouseEvent* event) {
    mView->mouseMoveEvent(event);
}

void WindowWidget::mousePressEvent(QMouseEvent* event) {
    mView->mousePressEvent(event);
}

void WindowWidget::mouseReleaseEvent(QMouseEvent* event) {
    mView->mouseReleaseEvent(event);
}
