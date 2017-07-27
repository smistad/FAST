#ifndef LINE_RENDERER_HPP_
#define LINE_RENDERER_HPP_

#include "FAST/Visualization/Renderer.hpp"
#include "FAST/Data/Color.hpp"
#include "FAST/Data/Mesh.hpp"

namespace fast {

class FAST_EXPORT  LineRenderer : public Renderer {
    FAST_OBJECT(LineRenderer)
    public:
        void addInputConnection(DataPort::pointer port);
        void addInputConnection(DataPort::pointer port, Color color, float width);
        void setDefaultColor(Color color);
        void setDefaultLineWidth(float width);
        void setDefaultDrawOnTop(bool drawOnTop);
        void setDrawOnTop(DataPort::pointer input, bool drawOnTop);
        void setColor(DataPort::pointer input, Color color);
        void setWidth(DataPort::pointer input, float width);
        void draw();
    private:
        LineRenderer();

        float mDefaultLineWidth;
        Color mDefaultColor;
        bool mDefaultDrawOnTop;
        std::unordered_map<DataPort::pointer, float> mInputWidths;
        std::unordered_map<DataPort::pointer, Color> mInputColors;
        std::unordered_map<DataPort::pointer, bool> mInputDrawOnTop;
};

}

#endif
