#pragma once

#include "Window.hpp"
#include "View.hpp"
#include "Renderer.hpp"
#include "WindowWidget.hpp"

namespace fast {

/**
 * @brief A Window with 2 views
 */
class FAST_EXPORT DualViewWindow : public Window {
    FAST_OBJECT_V4(DualViewWindow)
    public:
        /**
         * @brief Create instance
         * @param bgcolor background color for both views
         * @param width Width of window, if 0 width is set automatically
         * @param height Height of window, if 0 height is set automatically
         * @param verticalMode Whether to display the two views vertically or horizontally
         * @return instance
         */
        FAST_CONSTRUCTOR(DualViewWindow,
                         Color, bgcolor, = Color::White(),
                         int, width, = 0,
                         int, height, = 0,
                         bool, verticalMode, = false
        );
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
        void setBackgroundColor(Color color);
        void addWidget(QWidget* widget);
        ~DualViewWindow();
        std::shared_ptr<DualViewWindow> connectLeft(std::shared_ptr<Renderer> renderer);
        std::shared_ptr<DualViewWindow> connectLeft(std::vector<std::shared_ptr<Renderer>> renderers);
        std::shared_ptr<DualViewWindow> connectRight(std::shared_ptr<Renderer> renderer);
        std::shared_ptr<DualViewWindow> connectRight(std::vector<std::shared_ptr<Renderer>> renderers);
        std::shared_ptr<DualViewWindow> connectTop(std::shared_ptr<Renderer> renderer);
        std::shared_ptr<DualViewWindow> connectTop(std::vector<std::shared_ptr<Renderer>> renderers);
        std::shared_ptr<DualViewWindow> connectBottom(std::shared_ptr<Renderer> renderer);
        std::shared_ptr<DualViewWindow> connectBottom(std::vector<std::shared_ptr<Renderer>> renderers);
        std::shared_ptr<DualViewWindow> connect(QWidget* widget);
    protected:
        void createLayout();
        bool mVerticalMode;
        QVBoxLayout* m_widgetLayout;
};

/**
 * @brief A Window with 2 views in 2D mode
 */
class FAST_EXPORT DualViewWindow2D : public DualViewWindow {
    FAST_OBJECT_V4(DualViewWindow2D)
    public:
        /**
         * Create a DualViewWindow 2D object
         *
         * Args:
         * @param bgcolor Background color
         * @param width in pixels
         * @param height in pixels
         * @return shared ptr of new SimpleWindow
         */
        FAST_CONSTRUCTOR(DualViewWindow2D,
                         Color, bgcolor, = Color::White(),
                         int, width, = 0,
                         int, height, = 0,
                         bool, verticalMode, = false
        )
    };

/**
 * @brief A Window with 2 views in 3D mode
 */
class FAST_EXPORT DualViewWindow3D : public DualViewWindow {
    FAST_OBJECT_V4(DualViewWindow3D)
    public:
        /**
         * Create a DualViewWindow 3D object
         *
         * Args:
         * @param bgcolor Background color
         * @param width in pixels
         * @param height in pixels
         * @return shared ptr of new SimpleWindow
         */
        FAST_CONSTRUCTOR(DualViewWindow3D,
                         Color, bgcolor, = Color::White(),
                         int, width, = 0,
                         int, height, = 0,
                         bool, verticalMode, = false
        )
};

} // end namespace fast

