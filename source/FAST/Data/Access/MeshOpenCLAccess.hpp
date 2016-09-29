#ifndef MESH_OPENCL_ACCESS_HPP_
#define MESH_OPENCL_ACCESS_HPP_

#include "CL/OpenCL.hpp"
#include "FAST/SmartPointers.hpp"

namespace fast {

class Mesh;

class MeshOpenCLAccess {
public:
    typedef UniquePointer<MeshOpenCLAccess> pointer;
    MeshOpenCLAccess(cl::Buffer* coordinatesBuffer, cl::Buffer* connectionsBuffer, SharedPointer<Mesh> mesh);
    cl::Buffer* getCoordinatesBuffer() const;
    cl::Buffer* getConnectionsBuffer() const;
    void release();
    ~MeshOpenCLAccess();
private:
    MeshOpenCLAccess(const MeshOpenCLAccess& other);
    MeshOpenCLAccess& operator=(const MeshOpenCLAccess& other);
    cl::Buffer* mCoordinates;
    cl::Buffer* mConnections;
    SharedPointer<Mesh> mMesh;
    bool mIsDeleted;
};

}

#endif