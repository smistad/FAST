#ifndef SLICERENDERER_HPP_
#define SLICERENDERER_HPP_

#include "Renderer.hpp"
#include "ImageData.hpp"

namespace fast {

class SliceRenderer : public Renderer {
    FAST_OBJECT(SliceRenderer)
    public:
        void setInput(ImageData::pointer image);
    private:
        SliceRenderer();
        void execute();
        void draw();

        OpenCLDevice::pointer mDevice;
        ImageData::pointer mInput;
#if defined(CL_VERSION_1_2)
        cl::ImageGL mImageGL;
#else
        cl::Image2DGL mImageGL;
#endif
        GLuint mTexture;
        cl::Program mProgram;
        bool mTextureIsCreated;

};

}




#endif /* SLICERENDERER_HPP_ */
