#include "WindowWidget.hpp"
#include "SimpleWindow.hpp"

namespace fast {

void WindowWidget::keyPressEvent(QKeyEvent *event) {
    // Close window if user presses Q or escape
    switch (event->key()) {
        case Qt::Key_Q:
        case Qt::Key_Escape:
            Reporter::info() << "Close key pressed" << Reporter::end();
            close();
            return;
            break;
    }
    for (View *view : mViews)
        view->keyPressEvent(event);
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

View *WindowWidget::addView() {
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

}
