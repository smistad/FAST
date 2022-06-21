#pragma once

#include "FAST/Visualization/Renderer.hpp"
#include "FAST/Data/Color.hpp"
#include <mutex>

namespace fast {

class View;
class Text;

/**
 * @brief Renders Text data
 *
 * Renders text data objects.
 *
 * @ingroup renderers
 */
class FAST_EXPORT  TextRenderer : public Renderer {
    FAST_PROCESS_OBJECT(TextRenderer)
    public:
        DataBoundingBox getBoundingBox(bool transform) override;
        enum TextStyleType {
            STYLE_NORMAL,
            STYLE_BOLD,
            STYLE_ITALIC
        };
        enum TextPosition {
            POSITION_CENTER,
            POSITION_TOP_CENTER,
            POSITION_BOTTOM_CENTER,
            POSITION_BOTTOM_LEFT,
            POSITION_BOTTOM_RIGHT,
            POSITION_TOP_LEFT,
            POSITION_TOP_RIGHT
        };
        enum class PositionType {
                STANDARD = 0,
                VIEW,
                WORLD
        };
        FAST_CONSTRUCTOR(TextRenderer,
                         uint, fontSize, = 28,
                         Color, color, = Color::Green(),
                         TextStyleType, type, = STYLE_NORMAL,
                         TextPosition, position, = POSITION_CENTER,
                         PositionType, positionType, = PositionType::STANDARD
        )
        void setPosition(TextPosition position);
        /**
         * Set text position in normalization view position (x,y 0-1)
         * @param position
         * @param centerPosition
         */
        void setViewPosition(Vector2f position, float centerPosition = true);
        /**
         * Set text position in world coordinates (millimeters)
         * @param position
         * @param centerPosition
         */
        void setWorldPosition(Vector2f position, float centerPosition = true);
        /**
         * Set font height in millimeters
         * @param heightInMillimeters
         */
        void setFontHeightInMM(float heightInMillimeters);
        void setPositionType(PositionType position);
        void setFontSize(uint fontSize);
        void setColor(Color color);
        void setStyle(TextStyleType);
        void
        draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar, bool mode2D, int viewWidth,
             int viewHeight) override;
        void loadAttributes();
    private:

        std::unordered_map<uint, uint> mTexturesToRender;
        std::unordered_map<uint, std::shared_ptr<Text>> mTextUsed;
        std::unordered_map<uint, uint> mVAO;
        std::unordered_map<uint, uint> mVBO;
        std::unordered_map<uint, uint> mEBO;

        Color mColor;
        uint mFontSize;
        TextStyleType mStyle;
        TextPosition m_position;
        Vector2f m_worldPosition;
        Vector2f m_viewPosition;
        float m_textHeightInMM = 5.0f;
        bool m_centerPosition = true;

        PositionType m_positionType;
};

} // end namespace fast

