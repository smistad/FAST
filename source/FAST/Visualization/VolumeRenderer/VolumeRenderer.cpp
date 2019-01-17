#include "VolumeRenderer.hpp"
#include <FAST/Data/Image.hpp>
#include <FAST/Utility.hpp>

namespace fast {

void VolumeRenderer::draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, bool mode2D) {
    reportInfo() << "Draw in volume renderer" << reportEnd();
    // Create a FBO
    if(m_FBO == -1)
        glGenFramebuffers(1, &m_FBO);

    // Bind the framebuffer to render to it
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);

    // Attach texture to framebuffer
    if(m_texture == -1) {
        glGenTextures(1, &m_texture);
        glBindTexture(GL_TEXTURE_2D, m_texture);

        auto data = make_uninitialized_unique<float[]>(512*512*4);
        for(int i = 0; i < 512*512; ++i) {
            data[i*4 + 0] = 1.0f;
            data[i*4 + 1] = 0.0f;
            data[i*4 + 2] = 0.0f;
            data[i*4 + 3] = 1.0f;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 512, 512, 0, GL_RGBA, GL_FLOAT, data.get());
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);
    }

    // Blit/copy the framebuffer to the default framebuffer (window)
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_FBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    // Get window/viewport size
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    glBlitFramebuffer(0, 0, 512, 512, viewport[0], viewport[1], viewport[2], viewport[3], GL_COLOR_BUFFER_BIT, GL_LINEAR);
}

VolumeRenderer::~VolumeRenderer() {
    if(m_FBO != -1)
        glDeleteFramebuffers(1, &m_FBO);
    if(m_texture != -1)
        glDeleteFramebuffers(1, &m_texture);
}

VolumeRenderer::VolumeRenderer() {
    createInputPort<Image>(0);

}

}