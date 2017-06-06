#ifndef SEGMENTATION_RENDERER_HPP_
#define SEGMENTATION_RENDERER_HPP_

#include "FAST/Visualization/Renderer.hpp"
#include "FAST/Data/Segmentation.hpp"
#include "FAST/Data/Color.hpp"
#include "FAST/Utility.hpp"
#include <unordered_map>
#include <mutex>

namespace fast {

class FAST_EXPORT  SegmentationRenderer : public Renderer {
    FAST_OBJECT(SegmentationRenderer)
    public:
        void addInputConnection(ProcessObjectPort port);
        BoundingBox getBoundingBox();
        void setColor(Segmentation::LabelType, Color);
        void setFillArea(Segmentation::LabelType, bool);
        void setFillArea(bool fillArea);
    private:
        SegmentationRenderer();
        void execute();
        void draw();
        void draw2D(
                cl::Buffer PBO,
                uint width,
                uint height,
                Affine3f pixelToViewportTransform,
                float PBOspacing,
                Vector2f translation
        );

        bool mColorsModified;
        bool mFillAreaModified;

        std::unordered_map<uint, Image::pointer> mImagesToRender;
        std::unordered_map<int, Color> mLabelColors;
        std::unordered_map<int, bool> mLabelFillArea;
        bool mFillArea;
        cl::Buffer mColorBuffer, mFillAreaBuffer;
        std::mutex mMutex;

};

} // end namespace fast

#endif
