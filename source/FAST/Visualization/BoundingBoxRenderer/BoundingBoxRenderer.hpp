#pragma once

#include <FAST/Visualization/LabelColorRenderer.hpp>
#include <FAST/Data/BoundingBox.hpp>
#include <unordered_map>
#include <map>

namespace fast {

class FAST_EXPORT BoundingBoxRenderer : public LabelColorRenderer {
    FAST_PROCESS_OBJECT(BoundingBoxRenderer)
	public:
        virtual ~BoundingBoxRenderer();
    protected:
        FAST_CONSTRUCTOR(BoundingBoxRenderer, (std::map<uint, Color>), labelColors, = {})
        void draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar, bool mode2D);

        std::unordered_map<uint, float> mInputWidths;
        std::unordered_map<uint, bool> mInputDrawOnTop;
        std::unordered_map<uint, uint> mVAO;
};

}

