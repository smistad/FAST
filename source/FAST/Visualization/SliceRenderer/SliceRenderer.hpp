#ifndef SLICERENDERER_HPP_
#define SLICERENDERER_HPP_

#include "FAST/Visualization/Renderer.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {


class FAST_EXPORT  SliceRenderer : public Renderer {
    FAST_OBJECT(SliceRenderer)
    public:
        void addInputConnection(DataPort::pointer port) override;
        void setSliceToRender(unsigned int sliceNr);
        void setSlicePlane(PlaneType plane);
        BoundingBox getBoundingBox() override;
        void setIntensityLevel(float level);
        float getIntensityLevel();
        void setIntensityWindow(float window);
        float getIntensityWindow();
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

        unsigned int mWidth;
        unsigned int mHeight;

        std::mutex mMutex;

        // Level and window intensities
        float mWindow;
        float mLevel;
};

}




#endif /* SLICERENDERER_HPP_ */
