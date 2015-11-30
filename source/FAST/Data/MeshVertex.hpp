#ifndef MESHVERTEX_HPP_
#define MESHVERTEX_HPP_

#include "DataTypes.hpp"
#include <vector>

namespace fast {

class MeshVertex {
    public:
        Vector3f position;
        Vector3f normal;
        std::vector<unsigned int> triangles;
};

} // end namespace fast

#endif /* SURFACEVERTEX_HPP_ */
