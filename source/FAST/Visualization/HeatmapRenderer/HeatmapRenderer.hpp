#ifndef FAST_HEATMAP_RENDERER_HPP_
#define FAST_HEATMAP_RENDERER_HPP_

#include "FAST/Visualization/Renderer.hpp"
#include "FAST/Data/Color.hpp"

namespace fast {

class Image;

class FAST_EXPORT  HeatmapRenderer : public Renderer {
    FAST_OBJECT(HeatmapRenderer);
    public:
        void addInputConnection(ProcessObjectPort port, Color color = Color::Red());
        BoundingBox getBoundingBox();
    private:
        HeatmapRenderer();
        void execute();
        void draw();
        void draw2D(
                cl::Buffer PBO,
                uint width,
                uint height,
                Eigen::Transform<float, 3, Eigen::Affine> pixelToViewportTransform,
                float PBOspacing,
                Vector2f translation
        );

        std::unordered_map<uint, SharedPointer<Image> > mImagesToRender;
        std::unordered_map<uint, Color> mColors;
        std::mutex mMutex;
};

}

#endif