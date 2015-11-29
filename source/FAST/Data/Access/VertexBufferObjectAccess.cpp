#include "VertexBufferObjectAccess.hpp"
#include "FAST/Data/Mesh.hpp"

namespace fast {

GLuint* VertexBufferObjectAccess::get() const {
    return mVBOID;
}

VertexBufferObjectAccess::VertexBufferObjectAccess(
        GLuint VBOID,
        SharedPointer<Mesh> mesh) {

    mVBOID = new GLuint;
    *mVBOID = VBOID;

    mIsDeleted = false;
    mMesh = mesh;
}

void VertexBufferObjectAccess::release() {
	mMesh->VBOAccessFinished();
    if(!mIsDeleted) {
        delete mVBOID;
        mIsDeleted = true;
    }
}

VertexBufferObjectAccess::~VertexBufferObjectAccess() {
    release();
}

} // end namespacefast
