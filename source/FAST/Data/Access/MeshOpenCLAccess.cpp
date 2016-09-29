#include "MeshOpenCLAccess.hpp"
#include "FAST/Data/Mesh.hpp"

namespace fast {
MeshOpenCLAccess::MeshOpenCLAccess(cl::Buffer* coordinatesBuffer, cl::Buffer* connectionsBuffer, SharedPointer<Mesh> mesh) {
    mCoordinates = new cl::Buffer(*coordinatesBuffer);
    mConnections = new cl::Buffer(*connectionsBuffer);
    mMesh = mesh;
    mIsDeleted = false;
}

cl::Buffer* MeshOpenCLAccess::getCoordinatesBuffer() const {
    return mCoordinates;
}

cl::Buffer* MeshOpenCLAccess::getConnectionsBuffer() const {
    return mConnections;
}

void MeshOpenCLAccess::release() {
    if(!mIsDeleted) {
        delete mCoordinates;
        delete mConnections;
        mCoordinates = new cl::Buffer(); // assign a new blank object
        mConnections = new cl::Buffer(); // assign a new blank object
        mIsDeleted = true;
    }
    mMesh->accessFinished();
}

MeshOpenCLAccess::~MeshOpenCLAccess() {
    if(!mIsDeleted)
        release();
}

} // end namespace