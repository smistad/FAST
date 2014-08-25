#ifndef SURFACERENDERER_HPP_
#define SURFACERENDERER_HPP_

#include "Renderer.hpp"
#include "Surface.hpp"

namespace fast {

class SurfaceRenderer : public Renderer {
    FAST_OBJECT(SurfaceRenderer)
    public:
        void setInput(SurfaceData::pointer image);
        void setThreshold(float threshold){};
        BoundingBox getBoundingBox();
    private:
        SurfaceRenderer();
        void execute();
        void draw();

        OpenCLDevice::pointer mDevice;
        SurfaceData::pointer mInput;
        Surface::pointer mSurfaceToRender;
};

} // namespace fast




#endif /* SURFACERENDERER_HPP_ */
