#ifndef SURFACERENDERER_HPP_
#define SURFACERENDERER_HPP_

#include "Renderer.hpp"
#include "Surface.hpp"

namespace fast {

class SurfaceRenderer : public Renderer {
    FAST_OBJECT(SurfaceRenderer)
    public:
        void setInput(Surface::pointer image);
        void setThreshold(float threshold){};
        void keyPressEvent(QKeyEvent* event);
        void mouseMoveEvent(QMouseEvent* event, View* view);
        void resizeEvent(QResizeEvent* event);
        BoundingBox getBoundingBox();
    private:
        SurfaceRenderer();
        void execute();
        void draw();

        OpenCLDevice::pointer mDevice;
        Surface::pointer mInput;

        float camX,camY,camZ;
        float rotationX,rotationY;
        //unsigned int windowWidth, windowHeight;
        float scalingFactorx, scalingFactory, scalingFactorz;
        float translationx, translationy, translationz;

        unsigned int mWidth, mHeight;
};

} // namespace fast




#endif /* SURFACERENDERER_HPP_ */
