#ifndef POINT_RENDERER_HPP_
#define POINT_RENDERER_HPP_

#include "Renderer.hpp"
#include "PointSet.hpp"

namespace fast {

class PointRenderer : public Renderer {
    FAST_OBJECT(PointRenderer)
    public:
        void setInput(PointSet::pointer points);
        void draw();
        BoundingBox getBoundingBox();
    private:
        PointRenderer();
        void execute();

        float mPointSize;
};

}

#endif
