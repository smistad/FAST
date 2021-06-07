#pragma once

#include "Window.hpp"
#include "View.hpp"
#include "Renderer.hpp"

namespace fast {

class FAST_EXPORT  SimpleWindow : public Window {
    FAST_OBJECT(SimpleWindow)
    public:
        FAST_CONSTRUCTOR(SimpleWindow, bool, mode2D, = false, Color, bgcolor, = Color::White(), uint, width, = 0, uint, height, = 0)
        void addRenderer(std::shared_ptr<Renderer> renderer);
        void removeAllRenderers();
        void setMaximumFramerate(unsigned int framerate);
        View* getView();
        ~SimpleWindow();
    protected:
        void init();

};

} // end namespace fast

