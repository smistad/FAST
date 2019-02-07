#ifndef LINE_RENDERER_HPP_
#define LINE_RENDERER_HPP_

#include "FAST/Visualization/Renderer.hpp"
#include "FAST/Data/Color.hpp"
#include "FAST/Data/Mesh.hpp"

namespace fast {

class FAST_EXPORT  LineRenderer : public Renderer {
    FAST_OBJECT(LineRenderer)
    public:
        uint addInputConnection(DataPort::pointer port) override;
        uint addInputConnection(DataPort::pointer port, Color color, float width);
        void setDefaultColor(Color color);
        void setDefaultLineWidth(float width);
        void setDefaultDrawOnTop(bool drawOnTop);
        void setDrawOnTop(uint inputNr, bool drawOnTop);
        void setColor(uint inputNr, Color color);
        void setWidth(uint inputNr, float width);
        void draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar, bool mode2D);
    protected:
        LineRenderer();

        float mDefaultLineWidth;
        Color mDefaultColor;
        bool mDefaultColorSet;
        bool mDefaultDrawOnTop;
        std::unordered_map<uint, float> mInputWidths;
        std::unordered_map<uint, Color> mInputColors;
        std::unordered_map<uint, bool> mInputDrawOnTop;
};

}

#endif
