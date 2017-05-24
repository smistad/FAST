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

class Mesh;

class FAST_EXPORT  VertexBufferObjectAccess {
    public:
        GLuint* get() const;
        VertexBufferObjectAccess(GLuint VBOID, SharedPointer<Mesh> mesh);
        void release();
        ~VertexBufferObjectAccess();
		typedef UniquePointer<VertexBufferObjectAccess> pointer;
    private:
        GLuint* mVBOID;
        bool mIsDeleted;
        SharedPointer<Mesh> mMesh;
};

} // end namespace fast



#endif /* VERTEXBUFFEROBJECTACCESS_HPP_ */
