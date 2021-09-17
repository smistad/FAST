#pragma once

#include <FAST/Data/DataTypes.hpp>
#include <FAST/Data/SimpleDataObject.hpp>
#include <FAST/Data/Transform.hpp>

namespace fast {


/**
 * \brief Defines the extent of a spatial data object.
 *
 * This 3D BoundingBox class is used to define the extent
 * of a data object, which is used when rendering to determine
 * placement of the view camera
*/
class FAST_EXPORT DataBoundingBox {
    public:
        DataBoundingBox(Vector3f pos, Vector3f size);
        DataBoundingBox(Vector3f size);
        // Create a bounding box from a set of coordinates
        DataBoundingBox(std::vector<Vector3f> coordinates);
        DataBoundingBox(MatrixXf corners);
        DataBoundingBox();
        MatrixXf getCorners() const;
        DataBoundingBox getTransformedBoundingBox(Transform::pointer transform) const;
        DataBoundingBox getTransformedBoundingBox(Affine3f transform) const;
        bool isInitialized() const;
    private:
        void initialize(std::vector<Vector3f>);
        void createCorners(Vector3f pos, Vector3f size);
        MatrixXf mCorners;
        bool mIsInitialized;

};


std::ostream &operator<<(std::ostream &os, DataBoundingBox &object);

} // end namespace fast

