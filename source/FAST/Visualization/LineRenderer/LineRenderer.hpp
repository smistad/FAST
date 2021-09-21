#pragma once

#include "FAST/Visualization/Renderer.hpp"
#include "FAST/Data/Color.hpp"
#include "FAST/Data/Mesh.hpp"

namespace fast {

/**
 * @brief Renders lines stored in Mesh data objects.
 *
 * @ingroup renderers
 */
class FAST_EXPORT  LineRenderer : public Renderer {
    FAST_PROCESS_OBJECT(LineRenderer)
    public:
        /**
         * @brief Create instance
         * @param color Color of lines to draw
         * @param drawOnTop Whether to draw on top of everything else or not. This disables the depth check in OpenGL
         * @return instance
         */
        FAST_CONSTRUCTOR(LineRenderer,
                         Color, color, = Color::Green(),
                         bool, drawOnTop, = false
        );
        uint addInputConnection(DataChannel::pointer port) override;
        uint addInputConnection(DataChannel::pointer port, Color color, float width);
        void setDefaultColor(Color color);
        void setDefaultLineWidth(float width);
        void setDefaultDrawOnTop(bool drawOnTop);
        void setDrawOnTop(uint inputNr, bool drawOnTop);
        void setColor(uint inputNr, Color color);
        void setWidth(uint inputNr, float width);
        void draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar, bool mode2D);
    protected:

        float mDefaultLineWidth;
        Color mDefaultColor;
        bool mDefaultColorSet;
        bool mDefaultDrawOnTop;
        std::unordered_map<uint, float> mInputWidths;
        std::unordered_map<uint, Color> mInputColors;
        std::unordered_map<uint, bool> mInputDrawOnTop;
        std::unordered_map<uint, uint> mVAO;
};

}

