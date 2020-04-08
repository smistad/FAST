#pragma once

#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>

namespace fast {

class FAST_EXPORT VectorFieldColorRenderer : public ImageRenderer {
    FAST_OBJECT(VectorFieldColorRenderer)
    public:
        /**
         * Set the maximum opacity for the color overlay.
         *
         * @param maxOpacity
         */
        void setMaxOpacity(float maxOpacity);
        void setMaxLength(float max);
    private:
        VectorFieldColorRenderer();
        void draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar, bool mode2D) override;

        float m_maxOpacity = 0.5f;
        float m_maxLength = -1.0;
};

}