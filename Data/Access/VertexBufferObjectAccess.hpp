#ifndef VERTEXBUFFEROBJECTACCESS_HPP_
#define VERTEXBUFFEROBJECTACCESS_HPP_

#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenGL/OpenGL.h>
#else
#include <GL/gl.h>
#endif

namespace fast {

class VertexBufferObjectAccess {
    public:
        GLuint* get() const;
        VertexBufferObjectAccess(GLuint VBOID, bool* accessFlag, bool* accessFlag2);
        void release();
        ~VertexBufferObjectAccess();
    private:
        GLuint* mVBOID;
        bool mIsDeleted;
        bool* mAccessFlag;
        bool* mAccessFlag2;
};

} // end namespace fast



#endif /* VERTEXBUFFEROBJECTACCESS_HPP_ */
