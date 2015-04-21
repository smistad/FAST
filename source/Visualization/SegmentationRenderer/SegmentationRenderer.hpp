#ifndef SEGMENTATION_RENDERER_HPP_
#define SEGMENTATION_RENDERER_HPP_

#include "Renderer.hpp"
#include "Segmentation.hpp"
#include "Color.hpp"
#include <boost/unordered_map.hpp>
#include <boost/thread/mutex.hpp>

namespace fast {

class SegmentationRenderer : public Renderer {
    FAST_OBJECT(SegmentationRenderer)
    public:
        void addInputConnection(ProcessObjectPort port);
        BoundingBox getBoundingBox();
        void setColor(Segmentation::LabelType, Color);
    private:
        SegmentationRenderer();
        void execute();
        void draw();
        void draw2D(cl::BufferGL PBO, uint width, uint height, Eigen::Transform<float, 3, Eigen::Affine> pixelToViewportTransform, float PBOspacing);

        bool mDoTransformations;
        bool mColorsModified;

        boost::unordered_map<uint, Image::pointer> mImagesToRender;
        boost::unordered_map<uint, GLuint> mTexturesToRender;
        boost::unordered_map<uint, Image::pointer> mImageUsed;
        boost::unordered_map<Segmentation::LabelType, Color> mLabelColors;
        cl::Buffer mColorBuffer;
        boost::mutex mMutex;

};

} // end namespace fast

#endif
