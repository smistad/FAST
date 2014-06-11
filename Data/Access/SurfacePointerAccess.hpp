#ifndef SURFACEPOINTERACCESS_HPP_
#define SURFACEPOINTERACCESS_HPP_

#include "SurfaceVertex.hpp"
#include <vector>
#include <DataTypes.hpp>

namespace fast {

class SurfacePointerAccess {
    public:
        SurfacePointerAccess(std::vector<SurfaceVertex>* vertices, std::vector<Uint<3> >* triangles, bool* accessFlag, bool* accessFlag2);
        void release();
        ~SurfacePointerAccess();
    private:
        bool* mAccessFlag;
        bool* mAccessFlag2;
};

} // end namespace fast


#endif /* SURFACEPOINTERACCESS_HPP_ */
