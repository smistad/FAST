#ifndef TEXT_RENDERER_HPP_
#define TEXT_RENDERER_HPP_

#include "FAST/Visualization/Renderer.hpp"
#include "FAST/Data/Color.hpp"
#include <mutex>
#include <FAST/Data/SimpleDataObject.hpp>

namespace fast {

class View;

FAST_SIMPLE_DATA_OBJECT(Text, std::string)

class FAST_EXPORT  TextRenderer : public Renderer {
    FAST_OBJECT(TextRenderer)
    public:
        BoundingBox getBoundingBox();
        void setView(View* view);
        enum TextStyleType {
            STYLE_NORMAL,
            STYLE_BOLD,
            STYLE_ITALIC
        };
	void setPosition(Vector2i position);
        void setFontSize(uint fontSize);
        void setColor(Color color);
        void setStyle(TextStyleType);

    void
    draw2D(cl::Buffer PBO, uint width, uint height, Affine3f pixelToViewportTransform,
           float PBOspacing, Vector2f translation) override;
    void loadAttributes();
private:
        TextRenderer();
        void execute();
        void draw();

        std::mutex mMutex;
        View* mView;

        std::string mText;
        Color mColor;
        Vector2i mPosition2D;
        uint mFontSize;
        TextStyleType mStyle;
};

} // end namespace fast

#endif
