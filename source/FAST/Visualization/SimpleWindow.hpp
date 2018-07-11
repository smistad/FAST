#ifndef SIMPLEWINDOW_HPP_
#define SIMPLEWINDOW_HPP_

#include "Window.hpp"
#include "View.hpp"
#include "Renderer.hpp"

namespace fast {

class FAST_EXPORT  SimpleWindow : public Window {
    FAST_OBJECT(SimpleWindow)
    public:
        void addRenderer(SharedPointer<Renderer> renderer);
        void removeAllRenderers();
        void setMaximumFramerate(unsigned int framerate);
        View* getView() const;
        ~SimpleWindow();
        void set2DMode();
        void set3DMode();
        SimpleWindow();
    protected:

};

} // end namespace fast




#endif /* SIMPLEWINDOW_HPP_ */
