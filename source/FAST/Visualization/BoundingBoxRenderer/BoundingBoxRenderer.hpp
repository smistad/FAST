#pragma once

#include <FAST/Visualization/LabelColorRenderer.hpp>
#include <FAST/Data/BoundingBox.hpp>
#include <unordered_map>

namespace fast {

class FAST_EXPORT BoundingBoxRenderer : public LabelColorRenderer {
    FAST_OBJECT(BoundingBoxRenderer)
	public:
        virtual ~BoundingBoxRenderer();
    protected:
        BoundingBoxRenderer();
        void draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar, bool mode2D);

        std::unordered_map<uint, float> mInputWidths;
        std::unordered_map<uint, bool> mInputDrawOnTop;
        std::unordered_map<uint, uint> mVAO;
};

}

