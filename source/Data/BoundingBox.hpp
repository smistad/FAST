#ifndef BOUNDINGBOX_HPP_
#define BOUNDINGBOX_HPP_

#include "DataTypes.hpp"
#include "LinearTransformation.hpp"

namespace fast {

class BoundingBox {
    public:
        BoundingBox(Float3 pos, Float3 size);
        BoundingBox(Float3 size);
        // Create a bounding box from a set of coordinates
        BoundingBox(std::vector<Float3> coordinates);
        BoundingBox(Vector<Float3, 8> corners);
        BoundingBox();
        Vector<Float3, 8> getCorners();
        BoundingBox getTransformedBoundingBox(LinearTransformation transform);
    private:
        void createCorners(Float3 pos, Float3 size);
        Vector<Float3, 8> mCorners;
        bool mIsInitialized;

};


std::ostream &operator<<(std::ostream &os, BoundingBox &object);

} // end namespace fast



#endif /* BOUNDINGBOX_HPP_ */
