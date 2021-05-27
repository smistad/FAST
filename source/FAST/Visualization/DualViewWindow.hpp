#pragma once

#include "Window.hpp"
#include "View.hpp"
#include "Renderer.hpp"
#include "WindowWidget.hpp"

namespace fast {

class FAST_EXPORT  DualViewWindow : public Window {
    FAST_OBJECT(DualViewWindow)
    public:
        void addRendererToTopLeftView(Renderer::pointer renderer);
        void addRendererToBottomRightView(Renderer::pointer renderer);
        void addRendererToLeftView(Renderer::pointer renderer);
        void addRendererToRightView(Renderer::pointer renderer);
        void addRendererToTopView(Renderer::pointer renderer);
        void addRendererToBottomView(Renderer::pointer renderer);
        void removeAllRenderers();
        View* getTopLeftView();
        View* getBottomRightView();
        void setHorizontalMode();
        void setVerticalMode();
        ~DualViewWindow();
    protected:
        DualViewWindow();
        void createLayout();

        bool mVerticalMode;
};

} // end namespace fast

