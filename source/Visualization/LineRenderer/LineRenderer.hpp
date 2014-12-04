#ifndef POINT_RENDERER_HPP_
#define POINT_RENDERER_HPP_

#include "Renderer.hpp"
#include "LineSet.hpp"
#include "Color.hpp"

namespace fast {

class LineRenderer : public Renderer {
    FAST_OBJECT(LineRenderer)
    public:
        void addInput(LineSet::pointer lines);
        void addInput(LineSet::pointer lines, Color color, float width);
        void setDefaultColor(Color color);
        void setDefaultLineWidth(float width);
        void setDefaultDrawOnTop(bool drawOnTop);
        void setDrawOnTop(DataObject::pointer input, bool drawOnTop);
        void setColor(DataObject::pointer input, Color color);
        void setWidth(DataObject::pointer input, float width);
        void draw();
        BoundingBox getBoundingBox();
    private:
        LineRenderer();
        void execute();

        float mDefaultLineWidth;
        Color mDefaultColor;
        bool mDefaultDrawOnTop;
        boost::unordered_map<DataObject::pointer, float> mInputWidths;
        boost::unordered_map<DataObject::pointer, Color> mInputColors;
        boost::unordered_map<DataObject::pointer, bool> mInputDrawOnTop;
};

}

#endif
