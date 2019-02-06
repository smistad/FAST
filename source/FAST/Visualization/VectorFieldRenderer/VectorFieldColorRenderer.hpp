#pragma once

#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>

namespace fast {

class VectorFieldColorRenderer : public ImageRenderer {
    FAST_OBJECT(VectorFieldColorRenderer)
    public:
        /**
         * Set the maximum opacity for the color overlay. Default 0.8f
         *
         * @param maxOpacity
         */
        void setMaxOpacity(float maxOpacity);
    private:
        VectorFieldColorRenderer();
        void draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, bool mode2D) override;

        float m_maxOpacity = 0.8f;
};

}