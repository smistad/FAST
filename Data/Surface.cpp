#include "Surface.hpp"

namespace fast {

void Surface::create(
        std::vector<Float<3> > vertices,
        std::vector<Float<3> > normals,
        std::vector<Uint<3> > triangles) {
    mIsInitialized = true;

    for(unsigned int i = 0; i < vertices.size(); i++) {
        SurfaceVertex v;
        v.position = vertices[i];
        v.normal = normals[i];
        mVertices.push_back(v);
    }

    for(unsigned int i = 0; i < triangles.size(); i++) {
        mVertices[triangles[i].x()].triangles.push_back(i);
        mVertices[triangles[i].y()].triangles.push_back(i);
        mVertices[triangles[i].z()].triangles.push_back(i);
    }

    mNrOfTriangles = triangles.size();
}

void Surface::create(unsigned int nrOfTriangles) {
    mIsInitialized = true;
    mNrOfTriangles = nrOfTriangles;
}

bool Surface::isAnyDataBeingAccessed() {
    return mVBODataIsBeingAccessed || mHostDataIsBeingAccessed;
}

VertexBufferObjectAccess Surface::getVertexBufferObjectAccess(
        accessType type,
        ExecutionDevice::pointer device) {
    if(!mIsInitialized)
        throw Exception("Surface has not been initialized.");

    if(mSurfaceIsBeingWrittenTo)
        throw Exception("Requesting access to an image that is already being written to.");
    if (type == ACCESS_READ_WRITE) {
        if (isAnyDataBeingAccessed()) {
            throw Exception(
                    "Trying to get write access to an object that is already being accessed");
        }
        mSurfaceIsBeingWrittenTo = true;
    }
    if(!mVBOHasData) {
        // TODO create VBO

        // TODO Transfer data if any exist

    } else {
        if(!mVBODataIsUpToDate) {
            // TODO Update data
        }
    }

    mVBODataIsBeingAccessed = true;

    return VertexBufferObjectAccess(mVBOID, &mVBODataIsBeingAccessed, &mSurfaceIsBeingWrittenTo);
}

SurfacePointerAccess Surface::getSurfacePointerAccess(accessType access) {
    if(!mIsInitialized)
        throw Exception("Surface has not been initialized.");

    throw Exception("Not implemented yet!");
}

Surface::~Surface() {
    freeAll();
}

Surface::Surface() {
    mIsInitialized = false;
    mVBOHasData = false;
    mHostHasData = false;
    mSurfaceIsBeingWrittenTo = false;
}

void Surface::freeAll() {
}

void Surface::free(ExecutionDevice::pointer device) {
}

} // end namespace fast
