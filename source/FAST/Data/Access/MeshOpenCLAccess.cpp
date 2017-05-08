#include "MeshOpenCLAccess.hpp"
#include "FAST/Data/Mesh.hpp"

namespace fast {
MeshOpenCLAccess::MeshOpenCLAccess(
        cl::Buffer* coordinatesBuffer,
        cl::Buffer* lineBuffer,
        cl::Buffer* triangleBuffer,
        SharedPointer<Mesh> mesh
    ) {
    mCoordinates = new cl::Buffer(*coordinatesBuffer);
    if(lineBuffer != nullptr) {
        mLineBuffer = new cl::Buffer(*lineBuffer);
    } else {
        mLineBuffer = nullptr;
    }

    if(triangleBuffer != nullptr) {
        mTriangleBuffer = new cl::Buffer(*triangleBuffer);
    } else {
        mTriangleBuffer = nullptr;
    }
    mMesh = mesh;
    mIsDeleted = false;
}

cl::Buffer* MeshOpenCLAccess::getCoordinatesBuffer() const {
    return mCoordinates;
}

cl::Buffer* MeshOpenCLAccess::getLineBuffer() const {
    if(mLineBuffer == nullptr)
        throw Exception("Unable to get line buffer because mesh has no lines");
    return mLineBuffer;
}

cl::Buffer* MeshOpenCLAccess::getTriangleBuffer() const {
    if(mTriangleBuffer == nullptr)
        throw Exception("Unable to get triangle buffer because mesh has no triangles");
    return mTriangleBuffer;
}

void MeshOpenCLAccess::release() {
    if(!mIsDeleted) {
        delete mCoordinates;
        delete mLineBuffer;
        delete mTriangleBuffer;
        mCoordinates = new cl::Buffer(); // assign a new blank object
        mLineBuffer = new cl::Buffer(); // assign a new blank object
        mTriangleBuffer = new cl::Buffer(); // assign a new blank object
        mIsDeleted = true;
    }
    mMesh->accessFinished();
}

MeshOpenCLAccess::~MeshOpenCLAccess() {
    if(!mIsDeleted)
        release();
}

} // end namespace