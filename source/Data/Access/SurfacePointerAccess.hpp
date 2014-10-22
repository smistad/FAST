#ifndef SURFACEPOINTERACCESS_HPP_
#define SURFACEPOINTERACCESS_HPP_

#include "SurfaceVertex.hpp"
#include <vector>
#include <DataTypes.hpp>

namespace fast {

class SurfacePointerAccess {
    public:
        SurfacePointerAccess(std::vector<SurfaceVertex>* vertices, std::vector<Uint3>* triangles, bool* accessFlag, bool* accessFlag2);
        SurfaceVertex getVertex(uint i);
        Uint3 getTriangle(uint i);
        std::vector<Uint3> getTriangles();
        std::vector<SurfaceVertex> getVertices();
        void release();
        ~SurfacePointerAccess();
    private:
        bool* mAccessFlag;
        bool* mAccessFlag2;
        std::vector<SurfaceVertex>* mVertices;
        std::vector<Uint3>* mTriangles;
};

} // end namespace fast


#endif /* SURFACEPOINTERACCESS_HPP_ */
