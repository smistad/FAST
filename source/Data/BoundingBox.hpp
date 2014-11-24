#ifndef BOUNDINGBOX_HPP_
#define BOUNDINGBOX_HPP_

#include "DataTypes.hpp"
#include "LinearTransformation.hpp"

namespace fast {

class BoundingBox {
    public:
        BoundingBox(Vector3f pos, Vector3f size);
        BoundingBox(Vector3f size);
        // Create a bounding box from a set of coordinates
        BoundingBox(std::vector<Float3> coordinates);
        BoundingBox(std::vector<Vector3f> coordinates);
        BoundingBox(MatrixXf corners);
        BoundingBox();
        MatrixXf getCorners();
        BoundingBox getTransformedBoundingBox(LinearTransformation transform);
    private:
        void createCorners(Vector3f pos, Vector3f size);
        MatrixXf mCorners;
        bool mIsInitialized;

};


std::ostream &operator<<(std::ostream &os, BoundingBox &object);

} // end namespace fast



#endif /* BOUNDINGBOX_HPP_ */
