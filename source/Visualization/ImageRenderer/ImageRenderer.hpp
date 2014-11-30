#ifndef IMAGERENDERER_HPP_
#define IMAGERENDERER_HPP_

#include "Renderer.hpp"
#include "Image.hpp"

namespace fast {

class ImageRenderer : public Renderer {
    FAST_OBJECT(ImageRenderer)
    public:
        void setInput(ImageData::pointer image);
        void addInput(ImageData::pointer image);
        BoundingBox getBoundingBox();
        void turnOffTransformations();
    private:
        ImageRenderer();
        void execute();
        void draw();

        OpenCLDevice::pointer mDevice;
        ImageData::pointer mInput;
#if defined(CL_VERSION_1_2)
        cl::ImageGL mImageGL;
#else
        cl::Image2DGL mImageGL;
#endif
        boost::unordered_map<uint, Image::pointer> mImagesToRender;
        boost::unordered_map<uint, GLuint> mTexturesToRender;
        bool mTextureIsCreated;

        cl::Kernel mKernel;

        bool mDoTransformations;

};

}




#endif /* IMAGERENDERER_HPP_ */
