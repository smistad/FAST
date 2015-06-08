#ifndef VERTEXBUFFEROBJECTACCESS_HPP_
#define VERTEXBUFFEROBJECTACCESS_HPP_

#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenGL/OpenGL.h>
#else
#if _WIN32
#include <windows.h>
#include <GL/gl.h>
#else
#include <GL/gl.h>
#endif
#endif

#include "FAST/SmartPointers.hpp"

namespace fast {

class VertexBufferObjectAccess {
    public:
        GLuint* get() const;
        VertexBufferObjectAccess(GLuint VBOID, bool* accessFlag, bool* accessFlag2);
        void release();
        ~VertexBufferObjectAccess();
		typedef UniquePointer<VertexBufferObjectAccess> pointer;
    private:
        GLuint* mVBOID;
        bool mIsDeleted;
        bool* mAccessFlag;
        bool* mAccessFlag2;
};

} // end namespace fast



#endif /* VERTEXBUFFEROBJECTACCESS_HPP_ */
