#ifndef SURFACE_HPP_
#define SURFACE_HPP_

#include "SpatialDataObject.hpp"
#include "DynamicData.hpp"
#include "FAST/Data/Access/Access.hpp"
#include <vector>
#include "FAST/Data/DataTypes.hpp"
#include "FAST/Data/Access/VertexBufferObjectAccess.hpp"
#include "FAST/Data/Access/SurfacePointerAccess.hpp"

namespace fast {

class Mesh : public SpatialDataObject {
    FAST_OBJECT(Mesh)
    public:
        void create(std::vector<Vector3f> vertices, std::vector<Vector3f> normals, std::vector<Vector3ui> triangles);
        void create(std::vector<SurfaceVertex> vertices, std::vector<Vector3ui> triangles);
        void create(unsigned int nrOfTriangles);
        VertexBufferObjectAccess::pointer getVertexBufferObjectAccess(accessType access, OpenCLDevice::pointer device);
        SurfacePointerAccess::pointer getSurfacePointerAccess(accessType access);
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

} // end namespace fast


#endif /* SURFACE_HPP_ */
