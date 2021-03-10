#include "OpenGLTextureAccess.hpp"
#include <FAST/Data/Image.hpp>

namespace fast {

uint OpenGLTextureAccess::get() const {
    return m_textureID;
}

OpenGLTextureAccess::OpenGLTextureAccess(uint textureID, std::shared_ptr<Image> object) {
    m_textureID = textureID;
    mImageObject = object;
}

void OpenGLTextureAccess::release() {
    mImageObject->accessFinished();
}

OpenGLTextureAccess::~OpenGLTextureAccess() {
    release();
}

}