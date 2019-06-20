#ifndef FAST_HEATMAP_RENDERER_HPP_
#define FAST_HEATMAP_RENDERER_HPP_

#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Data/Color.hpp"

namespace fast {

class Image;

class FAST_EXPORT  HeatmapRenderer : public ImageRenderer {
    FAST_OBJECT(HeatmapRenderer);
    public:
        void setMinConfidence(float confidence);
        void setMaxOpacity(float opacity);
        uint addInputConnection(DataPort::pointer port, Color color = Color::Red());
    private:
        HeatmapRenderer();
        void draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar, bool mode2D) override;

        std::unordered_map<uint, Color> mColors;
        float mMaxOpacity = 0.6;
        float mMinConfidence = 0.0f;
};

}

#endif