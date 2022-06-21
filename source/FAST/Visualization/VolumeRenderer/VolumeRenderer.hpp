#pragma once

#include <FAST/Visualization/Renderer.hpp>

namespace fast {

/**
 * @brief Abstract base class for volume renderers
 *
 * @ingroup renderers
 */
class FAST_EXPORT VolumeRenderer : public Renderer {
    public:
        typedef std::shared_ptr<VolumeRenderer> pointer;
        ~VolumeRenderer();
    protected:
        virtual void
        draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar, bool mode2D, int viewWidth,
             int viewHeight) = 0;
        VolumeRenderer();
        uint m_FBO = 0;
        uint m_texture = 0;
        cl::Image2D textureToCLimage(uint textureID, int width, int height, OpenCLDevice::pointer device, bool depth);
        cl::ImageGL textureToCLimageInterop(uint textureID, int width, int height, OpenCLDevice::pointer device, bool depth);
        std::tuple<uint, uint> resizeOpenGLTexture(int sourceFBO, int sourceTextureColor, int sourceTextureDepth, Vector2i gridSize, int width, int height);
};

}