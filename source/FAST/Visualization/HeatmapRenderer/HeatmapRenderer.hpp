#pragma once

#include "FAST/Visualization/Renderer.hpp"
#include "FAST/Data/Color.hpp"

namespace fast {

class Tensor;

class FAST_EXPORT HeatmapRenderer : public Renderer {
    FAST_OBJECT(HeatmapRenderer);
    public:
        void setMinConfidence(float confidence);
        void setMaxOpacity(float opacity);
        void setChannelColor(uint channel, Color color);
    protected:
        HeatmapRenderer();
        void drawTextures(Matrix4f &perspectiveMatrix, Matrix4f &viewingMatrix, bool mode2D);
        void draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar, bool mode2D) override;

        std::unordered_map<uint, Color> mColors;
        std::unordered_map<uint, uint> mTexturesToRender;
        std::unordered_map<uint, SharedPointer<Tensor>> mTensorUsed;
        /**
         * Timestamp used to generate texture
         */
        std::unordered_map<uint, uint64_t> mDataTimestamp;
        std::unordered_map<uint, uint> mVAO;
        std::unordered_map<uint, uint> mVBO;
        std::unordered_map<uint, uint> mEBO;
        float mMaxOpacity = 0.6;
        float mMinConfidence = 0.0f;
        cl::Buffer mColorBuffer;
        bool mColorsModified;
};

}
