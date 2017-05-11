#ifndef MESH_OPENCL_ACCESS_HPP_
#define MESH_OPENCL_ACCESS_HPP_

#include "CL/OpenCL.hpp"
#include "FAST/SmartPointers.hpp"

namespace fast {

class Mesh;

class FAST_EXPORT  MeshOpenCLAccess {
public:
    typedef UniquePointer<MeshOpenCLAccess> pointer;
    MeshOpenCLAccess(cl::Buffer* coordinatesBuffer, cl::Buffer* lineBuffer, cl::Buffer* triangleBuffer, SharedPointer<Mesh> mesh);
    cl::Buffer* getCoordinatesBuffer() const;
    cl::Buffer* getLineBuffer() const;
    cl::Buffer* getTriangleBuffer() const;
    void release();
    ~MeshOpenCLAccess();
private:
    MeshOpenCLAccess(const MeshOpenCLAccess& other);
    MeshOpenCLAccess& operator=(const MeshOpenCLAccess& other);
    cl::Buffer* mCoordinates;
    cl::Buffer* mLineBuffer;
    cl::Buffer* mTriangleBuffer;
    SharedPointer<Mesh> mMesh;
    bool mIsDeleted;
};

}

#endif