#ifndef IMAGERENDERER_HPP_
#define IMAGERENDERER_HPP_

#include "Renderer.hpp"
#include "Image.hpp"

namespace fast {

class ImageRenderer : public Renderer {
    FAST_OBJECT(ImageRenderer)
    public:
        void addInputConnection(ProcessObjectPort port);
        BoundingBox getBoundingBox();
        void turnOffTransformations();
    private:
        ImageRenderer();
        void execute();
        void draw();
        void draw2D(cl::BufferGL PBO, uint width, uint height, Matrix4f pixelToViewportTransform, float PBOspacing);

        boost::unordered_map<uint, Image::pointer> mImagesToRender;
        boost::unordered_map<uint, GLuint> mTexturesToRender;
        boost::unordered_map<uint, Image::pointer> mImageUsed;

        cl::Kernel mKernel;

        bool mDoTransformations;

        boost::mutex mMutex;

};

}




#endif /* IMAGERENDERER_HPP_ */
