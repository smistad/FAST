#ifndef MESHACCESS_HPP_
#define MESHACCESS_HPP_

#include <vector>
#include <FAST/Data/DataTypes.hpp>
#include "FAST/Data/MeshVertex.hpp"
#include "FAST/SmartPointers.hpp"

namespace fast {

class Mesh;

class MeshAccess {
    public:
        MeshAccess(std::vector<MeshVertex>* vertices, std::vector<Vector3ui>* triangles, SharedPointer<Mesh> mesh);
        MeshVertex getVertex(uint i);
        Vector3ui getTriangle(uint i);
        std::vector<Vector3ui> getTriangles();
        std::vector<MeshVertex> getVertices();
        void release();
        ~MeshAccess();
		typedef UniquePointer<MeshAccess> pointer;
    private:
        std::vector<MeshVertex>* mVertices;
        std::vector<Vector3ui>* mTriangles;
        SharedPointer<Mesh> mMesh;
};

} // end namespace fast


#endif /* MESHACCESS_HPP_ */
