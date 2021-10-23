#pragma once

#include "FAST/Visualization/Renderer.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

/**
 * @brief Renders 2D Image data objects, both in 2D and 3D.
 *
 * @ingroup renderers
 */
class FAST_EXPORT  ImageRenderer : public virtual Renderer {
    FAST_PROCESS_OBJECT(ImageRenderer)
    public:
        FAST_CONSTRUCTOR(ImageRenderer, float, level,= -1, float, window, = -1)
        void loadAttributes() override;
        void setIntensityLevel(float level);
        float getIntensityLevel();
        void setIntensityWindow(float window);
        float getIntensityWindow();
        ~ImageRenderer();
    protected:
        void draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar, bool mode2D);
        virtual void deleteAllTextures();


        std::unordered_map<uint, uint> mTexturesToRender;
        std::unordered_map<uint, Image::pointer> mImageUsed;
        /**
         * Timestamp used to generate texture
         */
        std::unordered_map<uint, uint64_t> mDataTimestamp;
        std::unordered_map<uint, uint> mVAO;
        std::unordered_map<uint, uint> mVBO;
        std::unordered_map<uint, uint> mEBO;

        cl::Kernel mKernel;

        // Level and window intensities
        float mWindow;
        float mLevel;

        void drawTextures(Matrix4f &perspectiveMatrix, Matrix4f &viewingMatrix, bool mode2D, bool useInterpolation = false, bool useWindowLevel = true);
};

}