#ifndef DUAL_VIEW_WINDOW_HPP_
#define DUAL_VIEW_WINDOW_HPP_

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
        void removeAllRenderers();
        View* getTopLeftView() const;
        View* getBottomRightView() const;
        void setHorizontalMode();
        void setVerticalMode();
        ~DualViewWindow();
    protected:
        DualViewWindow();
        void createLayout();

        bool mVerticalMode;
};

} // end namespace fast


#endif
