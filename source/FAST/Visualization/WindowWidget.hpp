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

class FAST_EXPORT  WindowWidget : public QWidget {
    Q_OBJECT
    public:
        WindowWidget();
        void keyPressEvent(QKeyEvent* event);
        void closeEvent(QCloseEvent* event);
        View* addView();
        std::vector<View*> getViews() const;
        void clearViews();
        ~WindowWidget();
    private:
        std::vector<View*> mViews;

    signals:
        void widgetHasClosed();


};
}; // end namespace fast


#endif /* WINDOWWIDGET_HPP_ */
