#pragma once

#include <FAST/Visualization/Renderer.hpp>
#include <FAST/Data/BoundingBox.hpp>
#include <FAST/Data/Color.hpp>
#include <unordered_map>

namespace fast {

class FAST_EXPORT BoundingBoxRenderer : public Renderer {
    FAST_OBJECT(BoundingBoxRenderer)
	public:
        void setLabelColor(int label, Color color);
        virtual ~BoundingBoxRenderer();
    protected:
        BoundingBoxRenderer();
        void draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar, bool mode2D);

        std::unordered_map<uint, float> mInputWidths;
        std::unordered_map<uint, bool> mInputDrawOnTop;
        std::unordered_map<uint, uint> mVAO;

        Color m_defaultColor = Color::Green();

        bool m_colorsModified = true;
        GLuint m_colorsUBO = 0;
        std::unordered_map<uchar, Color> m_labelColors;
};

}

