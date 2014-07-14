/*
 * WindowWidget.hpp
 *
 *  Created on: May 14, 2014
 *      Author: smistad
 */

#ifndef WINDOWWIDGET_HPP_
#define WINDOWWIDGET_HPP_

#include <QWidget>
#include <QEventLoop>
#include "View.hpp"

namespace fast {

class WindowWidget : public QWidget {
    Q_OBJECT
    public:
        void keyPressEvent(QKeyEvent* event);
        void mouseMoveEvent(QMouseEvent* event);
        void mousePressEvent(QMouseEvent* event);
        void mouseReleaseEvent(QMouseEvent* event);
        void wheelEvent(QWheelEvent* event);
        WindowWidget(View::pointer view) : mView(view) {mEventLoop = NULL;};
        void setEventLoop(QEventLoop* eventLoop) { mEventLoop = eventLoop; };
    private:
        View::pointer mView;
        QEventLoop *mEventLoop;
};
}; // end namespace fast


#endif /* WINDOWWIDGET_HPP_ */
