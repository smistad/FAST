#pragma once

#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Data/Color.hpp"

namespace fast {

class Tensor;

/**
 * @brief Renders Tensor data objects as heatmaps.
 *
 * @ingroup renderers
 */
class FAST_EXPORT HeatmapRenderer : public ImageRenderer {
    FAST_OBJECT(HeatmapRenderer);
    public:
        void setMinConfidence(float confidence);
        void setMaxOpacity(float opacity);
        void setChannelColor(uint channel, Color color);
        void setChannelHidden(uint channel, bool hide);
        void setInterpolation(bool useInterpolation);
        void loadAttributes() override;
    protected:
        HeatmapRenderer();
        void drawTextures(Matrix4f &perspectiveMatrix, Matrix4f &viewingMatrix, bool mode2D);
        void draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar, bool mode2D) override;

        std::unordered_map<uint, Color> mColors;
        std::unordered_map<uint, bool> mHide;
        std::unordered_map<uint, std::shared_ptr<Tensor>> mTensorUsed;

        float mMaxOpacity = 0.3;
        float mMinConfidence = 0.5f;
        cl::Buffer mColorBuffer;
        bool mColorsModified;
        bool mUseInterpolation = true;
};

}
