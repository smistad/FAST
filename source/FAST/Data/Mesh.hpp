#ifndef SURFACE_HPP_
#define SURFACE_HPP_

#include "SpatialDataObject.hpp"
#include "DynamicData.hpp"
#include "FAST/Data/Access/Access.hpp"
#include <vector>
#include "FAST/Data/DataTypes.hpp"
#include "FAST/Data/Access/VertexBufferObjectAccess.hpp"
#include "FAST/Data/Access/MeshAccess.hpp"
#include "FAST/Data/Access/MeshOpenCLAccess.hpp"
#include <condition_variable>
#include <unordered_map>

namespace fast {

class Mesh : public SpatialDataObject {
    FAST_OBJECT(Mesh)
    public:
        void create(std::vector<Vector3f> vertices, std::vector<Vector3f> normals);
        void create(std::vector<Vector3f> vertices, std::vector<Vector3f> normals, std::vector<VectorXui> triangles);
        void create(std::vector<Vector2f> vertices, std::vector<Vector2f> normals, std::vector<VectorXui> lines);
        void create(std::vector<MeshVertex> vertices, std::vector<VectorXui> connections = std::vector<VectorXui>());
        void create(unsigned int nrOfTriangles);
        VertexBufferObjectAccess::pointer getVertexBufferObjectAccess(accessType access, OpenCLDevice::pointer device);
        MeshAccess::pointer getMeshAccess(accessType access);
        MeshOpenCLAccess::pointer getOpenCLAccess(accessType access, OpenCLDevice::pointer device);
        unsigned int getNrOfTriangles() const;
        unsigned int getNrOfLines() const;
        unsigned int getNrOfVertices() const;
        uchar getDimensions() const;
        void setBoundingBox(BoundingBox box);
        ~Mesh();
    private:
        Mesh();
        void freeAll();
        void free(ExecutionDevice::pointer device);
        void setAllDataToOutOfDate();
        void updateOpenCLBufferData(OpenCLDevice::pointer device);

        bool mIsInitialized;
        uchar mDimensions;
        unsigned int mNrOfConnections;

        // VBO data
        bool mVBOHasData;
        bool mVBODataIsUpToDate;
        GLuint mVBOID;

        // Host data
        bool mHostHasData;
        bool mHostDataIsUpToDate;
        std::vector<MeshVertex> mVertices;
        std::vector<VectorXui> mConnections;

        // OpenCL buffer data
        std::unordered_map<OpenCLDevice::pointer, cl::Buffer*> mCoordinatesBuffers;
        std::unordered_map<OpenCLDevice::pointer, cl::Buffer*> mConnectionBuffers;
        std::unordered_map<OpenCLDevice::pointer, bool> mCLBuffersIsUpToDate;

        // Declare as friends so they can get access to the accessFinished methods
        friend class MeshAccess;
        friend class VertexBufferObjectAccess;
        friend class MeshOpenCLAccess;
};

} // end namespace fast


#endif /* SURFACE_HPP_ */
