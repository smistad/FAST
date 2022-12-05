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
    FAST_PROCESS_OBJECT(HeatmapRenderer);
    public:
        FAST_CONSTRUCTOR(HeatmapRenderer,
                         bool, hideChannelZero, = false,
                         bool, useInterpolation, = true,
                         float, minConfidence, = 0.5f,
                         float, maxOpacity, = 0.3f,
                         LabelColors, channelColors, = LabelColors()
        )
        void setMinConfidence(float confidence);
        float getMinConfidence() const;
        void setMaxOpacity(float opacity);
        float getMaxOpacity() const;
        void setChannelColor(uint channel, Color color);
        Color getChannelColor(uint channel);
        void setChannelHidden(uint channel, bool hide);
        bool getChannelHidden(uint channel);
        void setInterpolation(bool useInterpolation);
        bool getInterpolation() const;
        std::string attributesToString() override;
        void loadAttributes() override;
    protected:
        void drawTextures(std::unordered_map<uint, std::shared_ptr<SpatialDataObject>> dataToRender, Matrix4f &perspectiveMatrix, Matrix4f &viewingMatrix, bool mode2D);
        void
        draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar, bool mode2D, int viewWidth,
             int viewHeight) override;

        std::map<uint, Color> mColors = {
                {0, Color::Green()},
                {1, Color::Blue()},
                {2, Color::Red()},
                {3, Color::Yellow()},
                {4, Color::Cyan()},
                {5, Color::Magenta()},
                {6, Color::Brown()},
                {255, Color::Cyan()}
        };
        std::map<uint, bool> mHide;
        std::map<uint, std::shared_ptr<Tensor>> mTensorUsed;

        float mMaxOpacity = 0.3;
        float mMinConfidence = 0.5f;
        cl::Buffer mColorBuffer;
        bool mColorsModified;
        bool mUseInterpolation = true;
};

}
