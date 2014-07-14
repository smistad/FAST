#ifndef SURFACE_HPP_
#define SURFACE_HPP_

#include "DataObject.hpp"
#include "Access.hpp"
#include <vector>
#include "DataTypes.hpp"
#include "VertexBufferObjectAccess.hpp"
#include "SurfacePointerAccess.hpp"

namespace fast {

struct SurfaceBoundingBox {
    Float<3> offset;
    Float<3> size;
};

class Surface : public DataObject {
    FAST_OBJECT(Surface)
    public:
        void create(std::vector<Float<3> > vertices, std::vector<Float<3> > normals, std::vector<Uint<3> > triangles);
        void create(unsigned int nrOfTriangles);
        VertexBufferObjectAccess getVertexBufferObjectAccess(accessType access, OpenCLDevice::pointer device);
        SurfacePointerAccess getSurfacePointerAccess(accessType access);
        unsigned int getNrOfTriangles() const;
        SurfaceBoundingBox getSurfaceBoundingBox() const;
        void setBoundingBox(SurfaceBoundingBox box);
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
        std::vector<Uint<3> > mTriangles;

        bool mSurfaceIsBeingWrittenTo;
        bool isAnyDataBeingAccessed();

        SurfaceBoundingBox mBoundingBox;
};

} // end namespace fast


#endif /* SURFACE_HPP_ */
