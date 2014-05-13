#ifndef SLICERENDERER_HPP_
#define SLICERENDERER_HPP_

#include "Renderer.hpp"
#include "Image.hpp"

namespace fast {

enum PlaneType {PLANE_X, PLANE_Y, PLANE_Z};

class SliceRenderer : public Renderer {
    FAST_OBJECT(SliceRenderer)
    public:
        void setInput(ImageData::pointer image);
        void setSliceToRender(int sliceNr);
        void setSlicePlane(PlaneType plane);
        void keyPressEvent(QKeyEvent* event);
    private:
        SliceRenderer();
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

        cl::Kernel mKernel;
        DataType mTypeCLCodeCompiledFor;

        int mSliceNr;
        PlaneType mSlicePlane;

        float mScale;
        unsigned int mWidth;
        unsigned int mHeight;
};

}




#endif /* SLICERENDERER_HPP_ */
