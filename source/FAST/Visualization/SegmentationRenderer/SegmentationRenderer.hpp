#ifndef SEGMENTATION_RENDERER_HPP_
#define SEGMENTATION_RENDERER_HPP_

#include "FAST/Visualization/Renderer.hpp"
#include "FAST/Data/Segmentation.hpp"
#include "FAST/Data/Color.hpp"
#include <boost/unordered_map.hpp>
#include <boost/thread/mutex.hpp>

namespace fast {

class SegmentationRenderer : public Renderer {
    FAST_OBJECT(SegmentationRenderer)
    public:
        void addInputConnection(ProcessObjectPort port);
        BoundingBox getBoundingBox();
        void setColor(Segmentation::LabelType, Color);
        void setFillArea(bool fillArea);
    private:
        SegmentationRenderer();
        void execute();
        void draw();
        void draw2D(
                cl::BufferGL PBO,
                uint width,
                uint height,
                Eigen::Transform<float, 3, Eigen::Affine> pixelToViewportTransform,
                float PBOspacing,
                Vector2f translation
        );

        bool mColorsModified;

        boost::unordered_map<uint, Image::pointer> mImagesToRender;
        boost::unordered_map<uint, GLuint> mTexturesToRender;
        boost::unordered_map<uint, Image::pointer> mImageUsed;
        boost::unordered_map<Segmentation::LabelType, Color> mLabelColors;
        bool mFillArea;
        cl::Buffer mColorBuffer;
        boost::mutex mMutex;

};

} // end namespace fast

#endif
