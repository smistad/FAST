#pragma once

#include <FAST/Object.hpp>
#include <FAST/Data/DataTypes.hpp>


namespace fast {

class Image;

class FAST_EXPORT OpenGLTextureAccess {
    public:
        uint get() const;
        OpenGLTextureAccess(uint textureID, std::shared_ptr<Image> object);
        void release();
        ~OpenGLTextureAccess();
		typedef std::unique_ptr<OpenGLTextureAccess> pointer;
    private:
		OpenGLTextureAccess(const OpenGLTextureAccess& other);
		OpenGLTextureAccess& operator=(const OpenGLTextureAccess& other);
        uint m_textureID;
        bool mIsDeleted;
        std::shared_ptr<Image> mImageObject;

};

} // end namespace fast
