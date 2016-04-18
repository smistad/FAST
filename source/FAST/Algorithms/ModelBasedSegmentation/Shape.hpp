#ifndef SHAPE_HPP_
#define SHAPE_HPP_

#include "FAST/Data/Mesh.hpp"

namespace fast {

/**
 * Class used to represent a mesh shape.
 *
 */
class Shape : public Object {
	FAST_OBJECT(Shape)
	public:
		Mesh::pointer getMesh() const;
		void setMesh(Mesh::pointer mesh);
		Vector3f getCentroid();
	private:
		Shape();
		Mesh::pointer mMesh;
};

}

#endif
