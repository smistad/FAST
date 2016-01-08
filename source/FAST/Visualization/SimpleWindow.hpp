#ifndef SIMPLEWINDOW_HPP_
#define SIMPLEWINDOW_HPP_

#include "Window.hpp"
#include "View.hpp"
#include "Renderer.hpp"

namespace fast {

class SimpleWindow : public Window {
    FAST_OBJECT(SimpleWindow)
    public:
        void addRenderer(Renderer::pointer renderer);
        void removeAllRenderers();
        void setMaximumFramerate(unsigned int framerate);
        void setWindowSize(unsigned int w, unsigned int h);
        View* getView() const;
        ~SimpleWindow();
        void set2DMode();
        void set3DMode();
    protected:
        SimpleWindow();

};

} // end namespace fast




#endif /* SIMPLEWINDOW_HPP_ */
