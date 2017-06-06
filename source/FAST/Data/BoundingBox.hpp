#ifndef BOUNDINGBOX_HPP_
#define BOUNDINGBOX_HPP_

#include "FAST/AffineTransformation.hpp"
#include "FAST/Data/DataTypes.hpp"

namespace fast {

class FAST_EXPORT  BoundingBox {
    public:
        BoundingBox(Vector3f pos, Vector3f size);
        BoundingBox(Vector3f size);
        // Create a bounding box from a set of coordinates
        BoundingBox(std::vector<Vector3f> coordinates);
        BoundingBox(MatrixXf corners);
        BoundingBox();
        MatrixXf getCorners() const;
        BoundingBox getTransformedBoundingBox(AffineTransformation::pointer transform) const;
        bool isInitialized() const;
    private:
        void initialize(std::vector<Vector3f>);
        void createCorners(Vector3f pos, Vector3f size);
        MatrixXf mCorners;
        bool mIsInitialized;

};


std::ostream &operator<<(std::ostream &os, BoundingBox &object);

} // end namespace fast



#endif /* BOUNDINGBOX_HPP_ */
