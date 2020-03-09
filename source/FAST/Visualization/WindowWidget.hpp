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
        View* createView();
        void addView(View* view);
        std::vector<View*> getViews() const;
        void clearViews();
        ~WindowWidget();
        void saveScreenshotOnClose(std::string filename);
        void saveScreenshotOfViewsOnClose(std::string filename);
    protected:
        bool mSaveScreenshotOnClose;
        std::string mScreenshotFilename;
        bool mSaveScreenshotOfViewsOnClose;
        std::string mScreenshotViewsFilename;
    private:
        std::vector<View*> mViews;

    signals:
        void widgetHasClosed();


};
}; // end namespace fast


#endif /* WINDOWWIDGET_HPP_ */
