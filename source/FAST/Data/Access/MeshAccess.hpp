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
        MeshAccess(std::vector<MeshVertex>* vertices, std::vector<VectorXui>* connections, SharedPointer<Mesh> mesh);
        MeshVertex getVertex(uint i);
        Vector3ui getTriangle(uint i);
        Vector2ui getLine(uint i);
        std::vector<VectorXui> getTriangles();
        std::vector<VectorXui> getLines();
        std::vector<MeshVertex> getVertices();
        void release();
        ~MeshAccess();
		typedef UniquePointer<MeshAccess> pointer;
    private:
        std::vector<MeshVertex>* mVertices;
        std::vector<VectorXui>* mConnections;
        SharedPointer<Mesh> mMesh;
};

} // end namespace fast


#endif /* MESHACCESS_HPP_ */
