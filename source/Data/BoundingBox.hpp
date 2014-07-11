#ifndef BOUNDINGBOX_HPP_
#define BOUNDINGBOX_HPP_

#include "DataTypes.hpp"

namespace fast {

class BoundingBox {
    public:
        BoundingBox(Float3 pos, Float3 size);
        BoundingBox(Float3 size);
        BoundingBox(Vector<Float3, 8> corners);
        BoundingBox();
        Vector<Float3, 8> getCorners();
    private:
        Vector<Float3, 8> mCorners;
        bool mIsInitialized;

};

} // end namespace fast



#endif /* BOUNDINGBOX_HPP_ */
