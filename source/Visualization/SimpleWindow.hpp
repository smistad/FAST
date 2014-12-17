#ifndef SIMPLEWINDOW_HPP_
#define SIMPLEWINDOW_HPP_

#include "Object.hpp"
#include "View.hpp"
#include "Renderer.hpp"
#include "WindowWidget.hpp"
#include <QGLContext>
#include <QEventLoop>

namespace fast {

class SimpleWindow : public Object {
    FAST_OBJECT(SimpleWindow)
    public:
        void addRenderer(Renderer::pointer renderer);
        void setMaximumFramerate(unsigned int framerate);
        void start();
        void setWindowSize(unsigned int w, unsigned int h);
        // Makes the window close after a specific number of ms
        void setTimeout(unsigned int milliseconds);
        static QGLContext *mGLContext;
        static void initializeQtApp();
        View::pointer getView() const;
        ~SimpleWindow();
        void set2DMode();
        void set3DMode();
    private:
        SimpleWindow();
        View::pointer mView;

        WindowWidget* mWidget;

        unsigned int mWidth, mHeight;

        unsigned int mTimeout;

        QEventLoop* mEventLoop;


};

} // end namespace fast




#endif /* SIMPLEWINDOW_HPP_ */
