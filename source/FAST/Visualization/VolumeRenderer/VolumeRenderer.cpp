#include "VolumeRenderer.hpp"
#include <FAST/Data/Image.hpp>
#include <FAST/Utility.hpp>

namespace fast {

cl::Image2D VolumeRenderer::textureToCLimage(uint textureID, int width, int height, OpenCLDevice::pointer device, bool depth) {
    // TODO CL-GL interop
    int totalSize = width * height;
    if(!depth)
        totalSize *= 4;

    auto data = make_uninitialized_unique<float[]>(totalSize);
    glBindTexture(GL_TEXTURE_2D, textureID);
    if(depth) {
        glGetTexImage(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data.get());
    } else {
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, data.get());
    }
    auto image = cl::Image2D(
        device->getContext(),
        CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        depth ? cl::ImageFormat(CL_R, CL_FLOAT) : cl::ImageFormat(CL_RGBA, CL_FLOAT),
        width, height, 0,
        data.get()
    );

    return image;
}


cl::ImageGL VolumeRenderer::textureToCLimageInterop(uint textureID, int width, int height, OpenCLDevice::pointer device, bool depth) {
    // Create CL-GL image
    auto imageGL = cl::ImageGL(
            device->getContext(),
            CL_MEM_READ_ONLY,
            GL_TEXTURE_2D,
            0,
            textureID
    );

    return imageGL;
}

std::tuple<uint, uint> VolumeRenderer::resizeOpenGLTexture(int sourceFBO, int sourceTextureColor, int sourceTextureDepth, Vector2i gridSize, int width, int height) {
    uint targetFBO;
    glGenFramebuffers(1, &targetFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targetFBO);

    uint colorTexture, depthTexture;
    glGenTextures(1, &colorTexture);
    glGenTextures(1, &depthTexture);
    // Transfer texture data
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, gridSize.x(), gridSize.y(), 0, GL_RGBA, GL_FLOAT, nullptr);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, gridSize.x(), gridSize.y(), 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    // Set texture to FBO
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, sourceFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targetFBO);
    glBlitFramebuffer(0, 0, width, height, 0, 0, gridSize.x(), gridSize.y(), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    glFinish();
    glDeleteFramebuffers(1, &targetFBO);

    return std::make_tuple(colorTexture, depthTexture);

}

VolumeRenderer::~VolumeRenderer() {
    if(m_FBO != -1)
        glDeleteFramebuffers(1, &m_FBO);
    if(m_texture != -1)
        glDeleteFramebuffers(1, &m_texture);
}

VolumeRenderer::VolumeRenderer() {
    createInputPort(0);
    m_3Donly = true;

}

}