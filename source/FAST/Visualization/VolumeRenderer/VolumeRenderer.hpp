#pragma once

#include <FAST/Visualization/Renderer.hpp>

namespace fast {

class VolumeRenderer : public Renderer {
    FAST_OBJECT(VolumeRenderer)
    public:
        ~VolumeRenderer();
    protected:
        VolumeRenderer();
        void draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, bool mode2D);
        uint m_FBO = 0;
        uint m_texture = 0;
};

}