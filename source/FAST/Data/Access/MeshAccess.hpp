#ifndef MESHACCESS_HPP_
#define MESHACCESS_HPP_

#include <vector>
#include <FAST/Data/DataTypes.hpp>
#include "FAST/Data/MeshVertex.hpp"
#include "FAST/SmartPointers.hpp"

namespace fast {

class Mesh;

class FAST_EXPORT  MeshAccess {
    public:
        MeshAccess(
				std::vector<MeshVertex>* vertices,
				std::vector<MeshLine>* lines,
				std::vector<MeshTriangle>* triangles,
				SharedPointer<Mesh> mesh
		);
        MeshVertex getVertex(uint i);
        void setVertex(uint i, MeshVertex);
        void addVertex(MeshVertex v);
        MeshTriangle getTriangle(uint i);
		void setTriangle(uint i, MeshTriangle);
		void addTriangle(MeshTriangle t);
        MeshLine getLine(uint i);
		void setLine(uint i, MeshLine);
		void addLine(MeshLine l);
        std::vector<MeshTriangle> getTriangles();
        std::vector<MeshLine> getLines();
        std::vector<MeshVertex> getVertices();
        void release();
        ~MeshAccess();
		typedef UniquePointer<MeshAccess> pointer;
    private:
        std::vector<MeshVertex>* mVertices;
        std::vector<MeshLine>* mLines;
		std::vector<MeshTriangle>* mTriangles;
        SharedPointer<Mesh> mMesh;
};

} // end namespace fast


#endif /* MESHACCESS_HPP_ */
