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
        ~WindowWidget();
    protected:
        void dragEnterEvent(QDragEnterEvent *event) override;
        void dropEvent(QDropEvent *event) override;
    private:
    signals:
        void widgetHasClosed();
        void filesDropped(const QList<QString> &filenames);

};
} // end namespace fast
