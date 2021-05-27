#pragma once

#include "Window.hpp"
#include "View.hpp"
#include "Renderer.hpp"

namespace fast {

class FAST_EXPORT  SimpleWindow : public Window {
    FAST_OBJECT(SimpleWindow)
    public:
        void addRenderer(std::shared_ptr<Renderer> renderer);
        void removeAllRenderers();
        void setMaximumFramerate(unsigned int framerate);
        View* getView();
        ~SimpleWindow();
        SimpleWindow();
    protected:

};

} // end namespace fast

