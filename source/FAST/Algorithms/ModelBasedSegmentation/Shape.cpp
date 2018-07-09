#include "Shape.hpp"

namespace fast {

void Shape::setMesh(Mesh::pointer mesh) {
	mMesh = mesh;
}

Mesh::pointer Shape::getMesh() const {
	return mMesh;
}

Vector3f Shape::getCentroid() {
	if(!mMesh)
		throw Exception("Mesh must be set in Shape before getting centroid");

	MeshAccess::pointer access = mMesh->getMeshAccess(ACCESS_READ);
	Vector3f centroid = Vector3f::Zero();
	for(int i = 0; i < mMesh->getNrOfVertices(); ++i) {
		centroid += access->getVertex(i).getPosition();
	}

	centroid /= mMesh->getNrOfVertices();
	return centroid;
}

Shape::Shape() {

}

}
