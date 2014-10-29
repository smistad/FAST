#ifndef SURFACEVERTEX_HPP_
#define SURFACEVERTEX_HPP_

#include "DataTypes.hpp"
#include <vector>

namespace fast {

class SurfaceVertex {
    public:
        Vector3f position;
        Vector3f normal;
        std::vector<unsigned int> triangles;
};

} // end namespace fast

#endif /* SURFACEVERTEX_HPP_ */
