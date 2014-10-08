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

class SurfaceData : public virtual DataObject {
    public:
        typedef SharedPointer<SurfaceData> pointer;
        virtual ~SurfaceData() {};
};

class Surface : public SurfaceData {
    FAST_OBJECT(Surface)
    public:
        void create(std::vector<Float3> vertices, std::vector<Float3> normals, std::vector<Uint3> triangles);
        void create(std::vector<SurfaceVertex> vertices, std::vector<Uint3> triangles);
        void create(unsigned int nrOfTriangles);
        VertexBufferObjectAccess getVertexBufferObjectAccess(accessType access, OpenCLDevice::pointer device);
        SurfacePointerAccess getSurfacePointerAccess(accessType access);
        unsigned int getNrOfTriangles() const;
        unsigned int getNrOfVertices() const;
        void setBoundingBox(BoundingBox box);
        ~Surface();
    private:
        Surface();
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
        std::vector<Uint3> mTriangles;

        bool mSurfaceIsBeingWrittenTo;
        bool isAnyDataBeingAccessed();
};

class DynamicSurface : public SurfaceData, public DynamicData<Surface> {
    FAST_OBJECT(DynamicSurface)
};

} // end namespace fast


#endif /* SURFACE_HPP_ */
