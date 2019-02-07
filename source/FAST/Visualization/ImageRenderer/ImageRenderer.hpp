#ifndef IMAGERENDERER_HPP_
#define IMAGERENDERER_HPP_

#include "FAST/Visualization/Renderer.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

class FAST_EXPORT  ImageRenderer : public Renderer {
    FAST_OBJECT(ImageRenderer)
    public:
        void loadAttributes() override;
        void setIntensityLevel(float level);
        float getIntensityLevel();
        void setIntensityWindow(float window);
        float getIntensityWindow();
    protected:
        ImageRenderer();
        void draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar, bool mode2D);

        std::unordered_map<uint, uint> mTexturesToRender;
        std::unordered_map<uint, Image::pointer> mImageUsed;
        std::unordered_map<uint, uint> mVAO;

        cl::Kernel mKernel;

        // Level and window intensities
        float mWindow;
        float mLevel;

        void drawTextures(Matrix4f &perspectiveMatrix, Matrix4f &viewingMatrix, bool mode2D);
};

}




#endif /* IMAGERENDERER_HPP_ */
