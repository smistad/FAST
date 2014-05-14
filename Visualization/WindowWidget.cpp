#include "WindowWidget.hpp"
using namespace fast;


void WindowWidget::keyPressEvent(QKeyEvent* event) {
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
