#include "WindowWidget.hpp"
#include "SimpleWindow.hpp"
#include <QMimeData>
#include <QString>
#include <QList>

namespace fast {

void WindowWidget::keyPressEvent(QKeyEvent *event) {
    // Close window if user presses Q or escape
    switch(event->key()) {
        case Qt::Key_Escape:
            Reporter::info() << "Close key pressed" << Reporter::end();
            close();
            return;
            break;
    }
}

void WindowWidget::dragEnterEvent(QDragEnterEvent *event) {
    Reporter::info() << "Drag event received in window widget" << Reporter::end();
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}


void WindowWidget::dropEvent(QDropEvent *event) {
    Reporter::info() << "Drop event received in window widget" << Reporter::end();
    QList<QString> fileNames;
    foreach (const QUrl &url, event->mimeData()->urls()) {
        QString fileName = url.toLocalFile();
        fileNames.push_back(fileName);
        Reporter::info() << "Dropped file:" << fileName.toStdString() << Reporter::end();
    }
    emit filesDropped(fileNames);
}

void WindowWidget::closeEvent(QCloseEvent *event) {
    Reporter::info() << "Close event triggered" << Reporter::end();
    emit widgetHasClosed();
}

WindowWidget::~WindowWidget() {
    Reporter::info() << "DESTROYING window widget and all related views" << Reporter::end();
}

WindowWidget::WindowWidget() {
    setWindowTitle("FAST");
}

}
