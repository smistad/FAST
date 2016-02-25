#include "MeshShape.hpp"

namespace fast {

void MeshShape::setMesh(Mesh::pointer mesh) {
	mMesh = mesh;
}

Mesh::pointer MeshShape::getMesh() const {
	return mMesh;
}

MeshShape::MeshShape() {

}

}
