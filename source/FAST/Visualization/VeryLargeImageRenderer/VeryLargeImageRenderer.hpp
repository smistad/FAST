#pragma once

#include <FAST/Visualization/Renderer.hpp>

namespace fast {

class ImagePyramid;

class FAST_EXPORT VeryLargeImageRenderer : public Renderer {
    FAST_OBJECT(VeryLargeImageRenderer)
    public:
        void loadAttributes() override;
        void setIntensityLevel(float level);
        float getIntensityLevel();
        void setIntensityWindow(float window);
        float getIntensityWindow();
    private:
        VeryLargeImageRenderer();
        void draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar, bool mode2D);

        std::unordered_map<std::string, uint> mTexturesToRender;
        std::unordered_map<uint, SharedPointer<ImagePyramid>> mImageUsed;
        std::unordered_map<std::string, uint> mVAO;
        std::unordered_map<std::string, uint> mVBO;
        std::unordered_map<std::string, uint> mEBO;

        cl::Kernel mKernel;

        // Level and window intensities
        float mWindow;
        float mLevel;

        int mCurrentLevel = 0;
        float mCurrentTileScale = 0;

        void drawTextures(Matrix4f &perspectiveMatrix, Matrix4f &viewingMatrix, bool mode2D);
};

}