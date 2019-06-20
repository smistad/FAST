#ifndef BOUNDING_BOX_RENDERER_HPP_
#define BOUNDING_BOX_RENDERER_HPP_

#include "FAST/Visualization/Renderer.hpp"
#include "FAST/Data/BoundingBox.hpp"
#include <mutex>
#include <unordered_map>

namespace fast {

class FAST_EXPORT  BoundingBoxRenderer : public Renderer {
    FAST_OBJECT(BoundingBoxRenderer)
    private:
        BoundingBoxRenderer();
        void execute();
        void draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar, bool mode2D);
        BoundingBox getBoundingBox();

        std::unordered_map<uint, BoundingBox> mBoxesToRender;
};

}

#endif
