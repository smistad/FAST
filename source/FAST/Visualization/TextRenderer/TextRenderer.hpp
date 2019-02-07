#ifndef TEXT_RENDERER_HPP_
#define TEXT_RENDERER_HPP_

#include "FAST/Visualization/Renderer.hpp"
#include "FAST/Data/Color.hpp"
#include <mutex>
#include <FAST/Data/SimpleDataObject.hpp>
#include <FAST/Data/Text.hpp>

namespace fast {

class View;

class FAST_EXPORT  TextRenderer : public Renderer {
    FAST_OBJECT(TextRenderer)
    public:
        BoundingBox getBoundingBox(bool transform) override;
        void setView(View* view);
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
        void setPosition(TextPosition position);
        void setFontSize(uint fontSize);
        void setColor(Color color);
        void setStyle(TextStyleType);
        void draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar, bool mode2D) override;
        void loadAttributes();
    private:
        TextRenderer();
        void execute() override;

        View* mView = nullptr;

        std::unordered_map<uint, uint> mTexturesToRender;
        std::unordered_map<uint, Text::pointer> mTextUsed;
        std::unordered_map<uint, uint> mVAO;
        std::unordered_map<uint, DataObject::pointer> mDataToRender;

        Color mColor;
        uint mFontSize;
        TextStyleType mStyle;
        TextPosition mPosition;
};

} // end namespace fast

#endif
