#ifndef IMAGERENDERER_HPP_
#define IMAGERENDERER_HPP_

#include "Renderer.hpp"
#include "Image.hpp"

namespace fast {

class ImageRenderer : public Renderer {
    FAST_OBJECT(ImageRenderer)
    public:
        void setInput(ImageData::pointer image);
        void keyPressEvent(QKeyEvent* event);
    private:
        ImageRenderer();
        void execute();
        void draw();
        void recompileOpenCLCode(Image::pointer input);

        OpenCLDevice::pointer mDevice;
        ImageData::pointer mInput;
#if defined(CL_VERSION_1_2)
        cl::ImageGL mImageGL;
#else
        cl::Image2DGL mImageGL;
#endif
        GLuint mTexture;
        bool mTextureIsCreated;

        DataType mTypeCLCodeCompiledFor;
        cl::Kernel mKernel;

        float mScale;
        unsigned int mWidth, mHeight;

};

}




#endif /* IMAGERENDERER_HPP_ */
