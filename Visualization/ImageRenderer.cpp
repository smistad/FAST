#include "ImageRenderer.hpp"
using namespace fast;

void ImageRenderer::execute() {
    // Create OpenGL texture
    // Create CL-GL image
}

void ImageRenderer::draw() {

    glBindTexture(GL_TEXTURE_2D, texture);

    glBegin(GL_QUADS);
        glTexCoord2i(0, 1);
        glVertex3f(-1.0f, 1.0f, 0.0f);
        glTexCoord2i(1, 1);
        glVertex3f( 1.0f, 1.0f, 0.0f);
        glTexCoord2i(1, 0);
        glVertex3f( 1.0f,-1.0f, 0.0f);
        glTexCoord2i(0, 0);
        glVertex3f(-1.0f,-1.0f, 0.0f);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
}
