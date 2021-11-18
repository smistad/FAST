#pragma once

#include <FAST/Visualization/LabelColorRenderer.hpp>
#include <FAST/Data/BoundingBox.hpp>
#include <unordered_map>
#include <map>

namespace fast {

class FAST_EXPORT BoundingBoxRenderer : public LabelColorRenderer {
    FAST_PROCESS_OBJECT(BoundingBoxRenderer)
	public:
        FAST_CONSTRUCTOR(BoundingBoxRenderer,
             float, borderSize, = 1.0f,
             LabelColors, labelColors, = LabelColors()
        )
        void setBorderSize(float size);
        float getBorderSize() const;
        virtual ~BoundingBoxRenderer();
    protected:
        void draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar, bool mode2D);

        std::unordered_map<uint, float> mInputWidths;
        std::unordered_map<uint, bool> mInputDrawOnTop;
        std::unordered_map<uint, uint> mVAO;

        float m_borderSize;
};

}

