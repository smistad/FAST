#ifndef SHAPE_HPP_
#define SHAPE_HPP_

#include "FAST/Data/Mesh.hpp"

namespace fast {

/**
 * Abstract base class used to represent a shape.
 * This is passed from the ShapeModel to the AppearanceModel.
 * It is created by the shape model.
 * Is it really needed? Why not just pass the shape model and state matrix to the appearance model?
 *
 */
class Shape {
	public:
	private:
};

}

#endif
