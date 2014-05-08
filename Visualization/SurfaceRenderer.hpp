#ifndef SURFACERENDERER_HPP_
#define SURFACERENDERER_HPP_

#include "Renderer.hpp"
#include "ImageData.hpp"

namespace fast {

class SurfaceRenderer : public Renderer {
    FAST_OBJECT(SurfaceRenderer)
    public:
        void setInput(ImageData::pointer image);
        void setThreshold(float threshold);
    private:
        SurfaceRenderer();
        void execute();
        void draw();

        OpenCLDevice::pointer mDevice;
        ImageData::pointer mInput;
};

} // namespace fast




#endif /* SURFACERENDERER_HPP_ */
