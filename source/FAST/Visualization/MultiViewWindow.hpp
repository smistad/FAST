#pragma once

#include "Window.hpp"
#include "View.hpp"
#include "Renderer.hpp"
#include "WindowWidget.hpp"

namespace fast {

/**
 * @brief Window with multiple views
 */
class FAST_EXPORT MultiViewWindow : public Window {
    FAST_OBJECT_V4(MultiViewWindow)
    public:
        FAST_CONSTRUCTOR(MultiViewWindow,
                         int, viewCount,,
                         Color, bgcolor, = Color::White(),
                         int, width, = 0,
                         int, height, = 0,
                         bool, verticalMode, = false
        )
        void addView(View* view);
        void addRenderer(int viewIndex, Renderer::pointer renderer);
        void removeAllRenderers();
        void setHorizontalMode();
        void setVerticalMode();
        void setBackgroundColor(Color color);
        void start() override;
        ~MultiViewWindow();
        std::shared_ptr<MultiViewWindow> connect(int viewNr, std::shared_ptr<Renderer> renderer);
        std::shared_ptr<MultiViewWindow> connect(int viewNr, std::vector<std::shared_ptr<Renderer>> renderers);
    protected:
        void createLayout();

        bool mVerticalMode = false;
};

} // end namespace fast

