#ifndef SURFACE_HPP_
#define SURFACE_HPP_

#include "DataObject.hpp"
#include "DynamicData.hpp"
#include "Access.hpp"
#include <vector>
#include "DataTypes.hpp"
#include "VertexBufferObjectAccess.hpp"
#include "SurfacePointerAccess.hpp"

namespace fast {

class MeshData : public virtual DataObject {
    public:
        typedef SharedPointer<MeshData> pointer;
        virtual ~MeshData() {};
};

class Mesh : public MeshData {
    FAST_OBJECT(Mesh)
    public:
        void create(std::vector<Vector3f> vertices, std::vector<Vector3f> normals, std::vector<Vector3ui> triangles);
        void create(std::vector<SurfaceVertex> vertices, std::vector<Vector3ui> triangles);
        void create(unsigned int nrOfTriangles);
        VertexBufferObjectAccess getVertexBufferObjectAccess(accessType access, OpenCLDevice::pointer device);
        SurfacePointerAccess getSurfacePointerAccess(accessType access);
        unsigned int getNrOfTriangles() const;
        unsigned int getNrOfVertices() const;
        void setBoundingBox(BoundingBox box);
        ~Mesh();
    private:
        Mesh();
        void freeAll();
        void free(ExecutionDevice::pointer device);

        bool mIsInitialized;
        unsigned int mNrOfTriangles;

        // VBO data
        bool mVBOHasData;
        bool mVBODataIsUpToDate;
        bool mVBODataIsBeingAccessed;
        GLuint mVBOID;

        // Host data
        bool mHostHasData;
        bool mHostDataIsUpToDate;
        bool mHostDataIsBeingAccessed;
        std::vector<SurfaceVertex> mVertices;
        std::vector<Vector3ui> mTriangles;

        bool mSurfaceIsBeingWrittenTo;
        bool isAnyDataBeingAccessed();
};

class DynamicMesh : public MeshData, public DynamicData<Mesh> {
    FAST_OBJECT(DynamicMesh)
};

} // end namespace fast


#endif /* SURFACE_HPP_ */
