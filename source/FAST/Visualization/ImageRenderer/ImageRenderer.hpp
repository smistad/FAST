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
        /**
         * @brief Create instance
         *
         * @param level
         * @param window
         * @param opacity Draw image with an opacity. Useful if you want to draw an image on top of another
         * @param applyTransformationsIn2D In 2D transformations (rotation/translation) are typically not applied.
         *      by setting this to true they will be applied.
         * @return instance
         */
        FAST_CONSTRUCTOR(ImageRenderer,
                         float, level,= -1,
                         float, window, = -1,
                         float, opacity, = -1,
                         bool, applyTransformationsIn2D, = false
        )
        void loadAttributes() override;
        void setIntensityLevel(float level);
        float getIntensityLevel();
        void setIntensityWindow(float window);
        float getIntensityWindow();
        ~ImageRenderer();
    protected:
        void
        draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar, bool mode2D, int viewWidth,
             int viewHeight);
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
        float mWindow = -1;
        float mLevel = -1;
        float m_opacity = -1;
        bool m_applyTransformationsIn2D = false;

        void drawTextures(std::unordered_map<uint, std::shared_ptr<SpatialDataObject>> copy, Matrix4f &perspectiveMatrix, Matrix4f &viewingMatrix, bool mode2D, bool useInterpolation = false, bool useWindowLevel = true);
};

}