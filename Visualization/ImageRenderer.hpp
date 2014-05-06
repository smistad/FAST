#ifndef IMAGERENDERER_HPP_
#define IMAGERENDERER_HPP_

#include "Renderer.hpp"
#include "ImageData.hpp"

namespace fast {

class ImageRenderer : public Renderer {
    FAST_OBJECT(ImageRenderer)
    public:
        void setInput(ImageData::pointer image);
        void setIntensityLevel(float level);
        float getIntensityLevel();
        void setIntensityWindow(float window);
        float getIntensityWindow();
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
        GLuint mTexture;
        cl::Program mProgram;
        bool mTextureIsCreated;

        // Level and window intensities
        float mWindow;
        float mLevel;

};

}




#endif /* IMAGERENDERER_HPP_ */
