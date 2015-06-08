#ifndef WINDOW_HPP_
#define WINDOW_HPP_

#include "FAST/Object.hpp"
#include "WindowWidget.hpp"
#include <QGLContext>
#include <QEventLoop>
#include "ComputationThread.hpp"
#include "View.hpp"
#include <vector>

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
    protected:
        Window();
        View* createView();
        View* getView(uint i) const;
        std::vector<View*> getViews() const;
        static QGLContext* mMainGLContext;

        WindowWidget* mWidget;
        unsigned int mWidth, mHeight;
        unsigned int mTimeout;
        QEventLoop* mEventLoop;
        ComputationThread* mThread;
    public slots:
        void stop();


};

} // end namespace fast

#endif
