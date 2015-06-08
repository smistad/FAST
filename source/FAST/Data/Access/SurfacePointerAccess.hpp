#ifndef SURFACEPOINTERACCESS_HPP_
#define SURFACEPOINTERACCESS_HPP_

#include "FAST/Data/SurfaceVertex.hpp"
#include <vector>
#include <FAST/Data/DataTypes.hpp>
#include "FAST/SmartPointers.hpp"

namespace fast {

class SurfacePointerAccess {
    public:
        SurfacePointerAccess(std::vector<SurfaceVertex>* vertices, std::vector<Vector3ui>* triangles, bool* accessFlag, bool* accessFlag2);
        SurfaceVertex getVertex(uint i);
        Vector3ui getTriangle(uint i);
        std::vector<Vector3ui> getTriangles();
        std::vector<SurfaceVertex> getVertices();
        void release();
        ~SurfacePointerAccess();
		typedef UniquePointer<SurfacePointerAccess> pointer;
    private:
        bool* mAccessFlag;
        bool* mAccessFlag2;
        std::vector<SurfaceVertex>* mVertices;
        std::vector<Vector3ui>* mTriangles;
};

} // end namespace fast


#endif /* SURFACEPOINTERACCESS_HPP_ */
