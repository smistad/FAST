#ifndef WINDOW_HPP_
#define WINDOW_HPP_

#include "FASTExport.hpp"
#include "FAST/Object.hpp"
#include "WindowWidget.hpp"
#include "ComputationThread.hpp"
#include "View.hpp"
#include <vector>
#include <QGLContext>

class QGLContext;
class QEventLoop;
class QOffscreenSurface;

namespace fast {

class Window : public QObject, public Object {
    Q_OBJECT
    public:
        static void initializeQtApp();
        static QGLContext* getMainGLContext();
        // Makes the window close after a specific number of ms
        virtual void setTimeout(unsigned int milliseconds);
        ~Window();
        virtual void start();
        void startComputationThread();
        void stopComputationThread();
        void setWidth(uint width);
        void setHeight(uint height);
        void setSize(uint width, uint height);
        void enableMaximized();
        void disableMaximized();
        void enableFullscreen();
        void disableFullscreen();
        void setTitle(std::string);
        std::vector<View*> getViews() const;
        View* getView(uint i) const;
        static void cleanup();
    protected:
        Window();
        View* createView();
        static FAST_EXPORT QGLContext* mMainGLContext;

        WindowWidget* mWidget;
        unsigned int mWidth, mHeight;
        bool mFullscreen, mMaximized;
        unsigned int mTimeout;
        QEventLoop* mEventLoop;
        ComputationThread* mThread;
    public slots:
        void stop();


};

} // end namespace fast

#endif
