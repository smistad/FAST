#ifndef SIMPLEWINDOW_HPP_
#define SIMPLEWINDOW_HPP_

#include "Object.hpp"
#include "View.hpp"
#include "Renderer.hpp"
#include "WindowWidget.hpp"

namespace fast {

class SimpleWindow : public Object {
    FAST_OBJECT(SimpleWindow)
    public:
        void addRenderer(Renderer::pointer renderer);
        void setMaximumFramerate(unsigned int framerate);
        void runMainLoop();
        void setWindowSize(unsigned int w, unsigned int h);
    private:
        SimpleWindow();
        View::pointer mView;

        WindowWidget* mWidget;

        unsigned int mWidth, mHeight;
};


} // end namespace fast




#endif /* SIMPLEWINDOW_HPP_ */
