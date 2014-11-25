#ifndef SURFACERENDERER_HPP_
#define SURFACERENDERER_HPP_

#include "Renderer.hpp"
#include "Surface.hpp"

namespace fast {

class SurfaceRenderer : public Renderer {
    FAST_OBJECT(SurfaceRenderer)
    public:
        void setInput(MeshData::pointer image);
        void setThreshold(float threshold){};
        BoundingBox getBoundingBox();
        void setOpacity(float opacity);
    private:
        SurfaceRenderer();
        void execute();
        void draw();

        OpenCLDevice::pointer mDevice;
        MeshData::pointer mInput;
        Mesh::pointer mSurfaceToRender;
        float mOpacity;
};

} // namespace fast




#endif /* SURFACERENDERER_HPP_ */
