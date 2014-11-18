#include "WindowWidget.hpp"
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
    mView->keyPressEvent(event);
}

void WindowWidget::closeEvent(QCloseEvent* event) {
    // Make sure event is not quit twice
    if(!mView->hasQuit()) {
        // This event occurs when window is closed or timeout is reached
        mView->quit();
        mEventLoop->quit();
    }
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

void WindowWidget::wheelEvent(QWheelEvent* event) {
    mView->wheelEvent(event);
}
