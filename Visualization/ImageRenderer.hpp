#ifndef IMAGERENDERER_HPP_
#define IMAGERENDERER_HPP_

#include "Renderer.hpp"
#include "Image2D.hpp"

namespace fast {

class ImageRenderer : public Renderer {
    FAST_OBJECT(ImageRenderer)
    public:
        void setInput(Image2D::pointer image);
    private:
        ImageRenderer();
        void execute();
        void draw();

        OpenCLDevice::pointer mDevice;
        Image2D::pointer mInput;
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




#endif /* IMAGERENDERER_HPP_ */
