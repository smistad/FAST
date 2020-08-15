#include "VertexBufferObjectAccess.hpp"
#include "FAST/Data/Mesh.hpp"

namespace fast {

GLuint* VertexBufferObjectAccess::getCoordinateVBO() const {
    return mCoordinateVBO;
}

GLuint* VertexBufferObjectAccess::getNormalVBO() const {
    return mNormalVBO;
}

GLuint* VertexBufferObjectAccess::getColorVBO() const {
    return mColorVBO;
}

GLuint* VertexBufferObjectAccess::getLineEBO() const {
    return mLineEBO;
}

GLuint* VertexBufferObjectAccess::getTriangleEBO() const {
    return mTriangleEBO;
}

VertexBufferObjectAccess::VertexBufferObjectAccess(
        GLuint coordinateVBO,
        GLuint normalVBO,
        GLuint colorVBO,
        GLuint lineEBO,
        GLuint triangleEBO,
        bool useNormalVBO,
        bool useColorVBO,
        bool useEBO,
        std::shared_ptr<Mesh> mesh
        ) {
    mCoordinateVBO = new GLuint;
    *mCoordinateVBO = coordinateVBO;
    mNormalVBO = new GLuint;
    *mNormalVBO = normalVBO;
    mColorVBO = new GLuint;
    *mColorVBO = colorVBO;
    mLineEBO = new GLuint;
    *mLineEBO = lineEBO;
    mTriangleEBO = new GLuint;
    *mTriangleEBO = triangleEBO;

    mUseNormalVBO = useNormalVBO;
    mUseColorVBO = useColorVBO;
    mUseEBO = useEBO;

    mIsDeleted = false;
    mMesh = mesh;
}

void VertexBufferObjectAccess::release() {
	mMesh->accessFinished();
    if(!mIsDeleted) {
        delete mCoordinateVBO;
        delete mNormalVBO;
        delete mColorVBO;
        delete mLineEBO;
        delete mTriangleEBO;
        mIsDeleted = true;
    }
}

VertexBufferObjectAccess::~VertexBufferObjectAccess() {
    release();
}

bool VertexBufferObjectAccess::hasNormalVBO() const {
    return mUseNormalVBO;
}

bool VertexBufferObjectAccess::hasColorVBO() const {
    return mUseColorVBO;
}

bool VertexBufferObjectAccess::hasEBO() const {
    return mUseEBO;
}

} // end namespacefast
