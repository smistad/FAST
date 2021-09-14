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
    for(View *view : mViews)
        view->keyPressEvent(event);
}

void WindowWidget::changeEvent(QEvent* event) {
    if(event->type() == QEvent::WindowStateChange) {
        if(isMinimized()) {
            Reporter::info() << "Window minimized; turning OFF synchronized rendering" << Reporter::end();
			for(auto view : mViews) {
				view->setSynchronizedRendering(false);
			}
        } else {
            Reporter::info() << "Window not minimized; turning ON synchronized rendering" << Reporter::end();
			for(auto view : mViews) {
				view->setSynchronizedRendering(true);
			}
        }
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
    if(mSaveScreenshotOnClose) {
        Reporter::info() << "Saving screenshot of window to " << mScreenshotFilename << Reporter::end();
        QPixmap windowImage = grab();
        QPainter painter(&windowImage);

        // Paint each view on the proper place
        for(View* view : mViews) {
            QPoint pos = view->mapTo(this, view->pos());
            QImage viewImage = view->grabFrameBuffer();
            painter.drawPixmap(pos, QPixmap::fromImage(viewImage));
        }
        windowImage.save(mScreenshotFilename.c_str());
    }
    if(mSaveScreenshotOfViewsOnClose) {
        Reporter::info() << "Saving screenshot of views to " << mScreenshotViewsFilename << Reporter::end();
        int counter = 0;
        for(View* view : mViews) {
            std::string filename = mScreenshotViewsFilename;
            int pos = filename.rfind('.');
            filename = filename.insert(pos, "_view_" + std::to_string(counter));
            view->grabFrameBuffer().save(filename.c_str());
            counter++;
        }
    }
    Reporter::info() << "Close event triggered" << Reporter::end();
    emit widgetHasClosed();
}

WindowWidget::~WindowWidget() {
    Reporter::info() << "DESTROYING window widget and all related views" << Reporter::end();
    for (View *view : mViews) {
        delete view;
    }
}

WindowWidget::WindowWidget() {
    setWindowTitle("FAST");
    mSaveScreenshotOnClose = false;
    mSaveScreenshotOfViewsOnClose = false;
}

View *WindowWidget::createView() {
    View *view = new View();
    mViews.push_back(view);
    return view;
}

void WindowWidget::clearViews() {
    mViews.clear();
}

std::vector<View *> WindowWidget::getViews() const {
    return mViews;
}

void WindowWidget::saveScreenshotOnClose(std::string filename) {
    mSaveScreenshotOnClose = true;
    mScreenshotFilename = filename;
}

void WindowWidget::saveScreenshotOfViewsOnClose(std::string filename) {
    mSaveScreenshotOfViewsOnClose = true;
    mScreenshotViewsFilename = filename;
}

void WindowWidget::addView(View *view) {
    mViews.push_back(view);
}

}
