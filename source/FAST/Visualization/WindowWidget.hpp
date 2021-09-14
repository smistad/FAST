#pragma once

#include <QWidget>
#include <QEventLoop>
#include "View.hpp"
#include <QDragEnterEvent>
#include <QDropEvent>

namespace fast {

class FAST_EXPORT  WindowWidget : public QWidget {
    Q_OBJECT
    public:
        WindowWidget();
        void keyPressEvent(QKeyEvent* event);
        void closeEvent(QCloseEvent* event);
        void changeEvent(QEvent* event);
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
        void dragEnterEvent(QDragEnterEvent *event) override;
        void dropEvent(QDropEvent *event) override;
    private:
        std::vector<View*> mViews;

    signals:
        void widgetHasClosed();
        void filesDropped(const QList<QString> &filenames);

};
} // end namespace fast
