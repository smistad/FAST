#pragma once

#include "Window.hpp"
#include "View.hpp"
#include "Renderer.hpp"
#include "WindowWidget.hpp"

namespace fast {

class FAST_EXPORT  MultiViewWindow : public Window {
    FAST_OBJECT(MultiViewWindow)
    public:
        void addView(View* view);
        void setNrOfViews(int viewCount);
        void addRenderer(int viewIndex, Renderer::pointer renderer);
        void removeAllRenderers();
        void setHorizontalMode();
        void setVerticalMode();
        void start() override;
        ~MultiViewWindow();
    protected:
        MultiViewWindow();
        void createLayout();

        bool mVerticalMode;
};

} // end namespace fast

