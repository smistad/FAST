#ifndef POINT_RENDERER_HPP_
#define POINT_RENDERER_HPP_

#include "Renderer.hpp"
#include "PointSet.hpp"
#include "Color.hpp"

namespace fast {

class PointRenderer : public Renderer {
    FAST_OBJECT(PointRenderer)
    public:
        void addInput(PointSet::pointer points);
        void addInput(PointSet::pointer points, Color color, float size);
        void setDefaultColor(Color color);
        void setDefaultSize(float size);
        void setDefaultDrawOnTop(bool drawOnTop);
        void setDrawOnTop(DataObject::pointer input, bool drawOnTop);
        void setColor(DataObject::pointer input, Color color);
        void setSize(DataObject::pointer input, float size);
        void draw();
        BoundingBox getBoundingBox();
    private:
        PointRenderer();
        void execute();

        float mDefaultPointSize;
        Color mDefaultColor;
        bool mDefaultDrawOnTop;
        boost::unordered_map<DataObject::pointer, float> mInputSizes;
        boost::unordered_map<DataObject::pointer, Color> mInputColors;
        boost::unordered_map<DataObject::pointer, bool> mInputDrawOnTop;
};

}

#endif
