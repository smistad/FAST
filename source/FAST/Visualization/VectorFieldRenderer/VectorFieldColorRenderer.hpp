#pragma once

#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>

namespace fast {

/**
 * @brief Renders a vector field image using colors
 *
 * @ingroup renderers
 */
class FAST_EXPORT VectorFieldColorRenderer : public ImageRenderer {
    FAST_PROCESS_OBJECT(VectorFieldColorRenderer)
    public:
        FAST_CONSTRUCTOR(VectorFieldColorRenderer,
                         float, maxOpacity, = 0.5f,
                         float, maxLength, = -1.0f)
        /**
         * Set the maximum opacity for the color overlay.
         *
         * @param maxOpacity
         */
        void setMaxOpacity(float maxOpacity);
        void setMaxLength(float max);
    private:
        void
        draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar, bool mode2D, int viewWidth,
             int viewHeight) override;

        float m_maxOpacity = 0.5f;
        float m_maxLength = -1.0;
};

}