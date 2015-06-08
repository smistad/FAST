#include "VertexBufferObjectAccess.hpp"

namespace fast {

GLuint* VertexBufferObjectAccess::get() const {
    return mVBOID;
}

VertexBufferObjectAccess::VertexBufferObjectAccess(
        GLuint VBOID,
        bool* accessFlag,
        bool* accessFlag2) {

    mVBOID = new GLuint;
    *mVBOID = VBOID;

    mIsDeleted = false;
    mAccessFlag = accessFlag;
    mAccessFlag2 = accessFlag2;
}

void VertexBufferObjectAccess::release() {
    *mAccessFlag = false;
    *mAccessFlag2 = false;
    if(!mIsDeleted) {
        delete mVBOID;
        mIsDeleted = true;
    }
}

VertexBufferObjectAccess::~VertexBufferObjectAccess() {
    release();
}

} // end namespacefast
