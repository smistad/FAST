#ifndef MESH_OPENCL_ACCESS_HPP_
#define MESH_OPENCL_ACCESS_HPP_

#include "FAST/OpenCL.hpp"
#include "FAST/Object.hpp"


namespace fast {

class Mesh;

class FAST_EXPORT  MeshOpenCLAccess {
public:
    typedef std::unique_ptr<MeshOpenCLAccess> pointer;
    MeshOpenCLAccess(cl::Buffer* coordinatesBuffer, cl::Buffer* lineBuffer, cl::Buffer* triangleBuffer, std::shared_ptr<Mesh> mesh);
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
    std::shared_ptr<Mesh> mMesh;
    bool mIsDeleted;
};

}

#endif