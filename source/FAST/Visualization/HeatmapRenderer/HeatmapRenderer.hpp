#ifndef FAST_HEATMAP_RENDERER_HPP_
#define FAST_HEATMAP_RENDERER_HPP_

#include "FAST/Visualization/Renderer.hpp"
#include "FAST/Data/Color.hpp"

namespace fast {

class Image;

class FAST_EXPORT  HeatmapRenderer : public Renderer {
    FAST_OBJECT(HeatmapRenderer);
    public:
        void setMinConfidence(float confidence);
        void setMaxOpacity(float opacity);
        uint addInputConnection(DataPort::pointer port, Color color = Color::Red());
    private:
        HeatmapRenderer();
        void draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix) override;
        void draw2D(
                cl::Buffer PBO,
                uint width,
                uint height,
                Affine3f pixelToViewportTransform,
                float PBOspacing,
                Vector2f translation
        ) override;

        std::unordered_map<uint, Color> mColors;
        float mMaxOpacity = 0.4;
        float mMinConfidence = 0.0f;
};

}

#endif