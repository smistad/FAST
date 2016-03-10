#ifndef SLICERENDERER_HPP_
#define SLICERENDERER_HPP_

#include "FAST/Visualization/Renderer.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {


class SliceRenderer : public Renderer {
    FAST_OBJECT(SliceRenderer)
    public:
        void setInputConnection(ProcessObjectPort port);
        void setSliceToRender(unsigned int sliceNr);
        void setSlicePlane(PlaneType plane);
        BoundingBox getBoundingBox();
    private:
        SliceRenderer();
        void execute();
        void draw();
        void recompileOpenCLCode(Image::pointer input);

        Image::pointer mImageToRender;
#if defined(CL_VERSION_1_2)
        cl::ImageGL mImageGL;
#else
        cl::Image2DGL mImageGL;
#endif
        GLuint mTexture;
        bool mTextureIsCreated;

        cl::Kernel mKernel;
        DataType mTypeCLCodeCompiledFor;

        unsigned int mSliceNr;
        PlaneType mSlicePlane;

        float mScale;
        unsigned int mWidth;
        unsigned int mHeight;

        boost::mutex mMutex;
};

}




#endif /* SLICERENDERER_HPP_ */
