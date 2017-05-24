#ifndef SLICERENDERER_HPP_
#define SLICERENDERER_HPP_

#include "FAST/Visualization/Renderer.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {


class FAST_EXPORT  SliceRenderer : public Renderer {
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
        GLuint mTexture;
        bool mTextureIsCreated;

        unsigned int mSliceNr;
        PlaneType mSlicePlane;

        float mScale;
        unsigned int mWidth;
        unsigned int mHeight;

        std::mutex mMutex;
};

}




#endif /* SLICERENDERER_HPP_ */
