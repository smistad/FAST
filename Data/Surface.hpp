#ifndef SURFACE_HPP_
#define SURFACE_HPP_

#include "DataObject.hpp"
#include "Access.hpp"
#include <vector>
#include "DataTypes.hpp"
#include "VertexBufferObjectAccess.hpp"
#include "SurfacePointerAccess.hpp"

namespace fast {



class Surface : public DataObject {
    FAST_OBJECT(Surface)
    public:
        void create(std::vector<Float<3> > vertices, std::vector<Float<3> > normals, std::vector<Uint<3> > triangles);
        void create(unsigned int nrOfTriangles);
        VertexBufferObjectAccess getVertexBufferObjectAccess(accessType access, ExecutionDevice::pointer device);
        SurfacePointerAccess getSurfacePointerAccess(accessType access);
        ~Surface();
    private:
        Surface();
        void freeAll();
        void free(ExecutionDevice::pointer device);

        bool mIsInitialized;
        unsigned int nrOfTriangles;

        // VBO data
        bool mVBOHasData;
        bool mVBODataIsUpToDate;
        bool mVBODataIsBeingAccessed;
        GLuint VBO_ID;

        // Host data
        bool mHostHasData;
        bool mHostDataIsUpToDate;
        bool mHostDataIsBeingAccessed;
        std::vector<SurfaceVertex> mVertices;
        std::vector<Uint<3> > mTriangles;
};

} // end namespace fast


#endif /* SURFACE_HPP_ */
