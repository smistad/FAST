#ifndef MULTI_VIEW_WINDOW_HPP_
#define MULTI_VIEW_WINDOW_HPP_

#include "Window.hpp"
#include "View.hpp"
#include "Renderer.hpp"
#include "WindowWidget.hpp"

namespace fast {

class FAST_EXPORT  MultiViewWindow : public Window {
    FAST_OBJECT(MultiViewWindow)
    public:
        void setNrOfViews(int viewCount);
        void addRenderer(int viewIndex, Renderer::pointer renderer);
        void removeAllRenderers();
        void setHorizontalMode();
        void setVerticalMode();
        ~MultiViewWindow();
    protected:
        MultiViewWindow();
        void createLayout();

        bool mVerticalMode;
};

} // end namespace fast


#endif
