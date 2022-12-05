#pragma once

#include <FAST/Visualization/LabelColorRenderer.hpp>
#include <FAST/Data/BoundingBox.hpp>
#include <unordered_map>
#include <map>

namespace fast {

/**
 * @brief Renders a set of bounding boxes
 *
 * @sa BoundingBoxSet
 *
 * @ingroup renderers
 */
class FAST_EXPORT BoundingBoxRenderer : public LabelColorRenderer {
    FAST_PROCESS_OBJECT(BoundingBoxRenderer)
	public:
        /**
         * @brief Create instance
         *
         * @param borderSize Size of bounding box lines. If zero or negative value, the 10% of the minimum size of
         *      BoundingBoxSet is used instead.
         * @param labelColors Color of each label/class
         * @return instance
         */
        FAST_CONSTRUCTOR(BoundingBoxRenderer,
             float, borderSize, = 0.0f,
             LabelColors, labelColors, = LabelColors()
        )
        void setBorderSize(float size);
        float getBorderSize() const;
        std::string attributesToString() override;
        void loadAttributes() override;
        virtual ~BoundingBoxRenderer();
    protected:
        void
        draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar, bool mode2D, int viewWidth,
             int viewHeight);

        std::unordered_map<uint, float> mInputWidths;
        std::unordered_map<uint, bool> mInputDrawOnTop;
        std::unordered_map<uint, uint> mVAO;

        float m_borderSize;
};

}

