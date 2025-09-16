#pragma once

#include <FAST/Data/DataTypes.hpp>
#include <FAST/Data/SimpleDataObject.hpp>
#include <FAST/Data/Transform.hpp>

namespace fast {


/**
 * @brief Defines the extent of a spatial data object.
 *
 * This 3D BoundingBox class is used to define the extent
 * of a data object, which is used when rendering to determine
 * placement of the view camera
 *
 * @ingroup data
*/
class FAST_EXPORT DataBoundingBox {
    public:
        /**
         * @brief Create a bounding box from an offset pos and size
         * @param pos offset
         * @param size size
         */
        DataBoundingBox(Vector3f pos, Vector3f size);
        /**
         * @brief Create bounding box from a size vector, assuming data starts at origo
         * @param size size
         */
        DataBoundingBox(Vector3f size);
        /**
         * @brief Create bounding box from a set of coordinates
         * @param coordinates
         */
        DataBoundingBox(const std::vector<Vector3f>& coordinates);
        /**
         * @brief Update bounding box from a set of coordinates
         * This will update the existing bounding box, if any of the coordinates are outside
         * the existing bounding box.
         * @param coordinates
         */
        void update(const std::vector<Vector3f>& coordinates);
        /**
         * @brief Create bounding box from a 8x3 matrix of corner coordinates
         * @param corners
         */
        DataBoundingBox(MatrixXf corners);
        /**
         * @brief Create an uninitialized bounding box
         */
        DataBoundingBox();
        /**
         * @brief Get bounding box corners
         * @return
         */
        MatrixXf getCorners() const;
        DataBoundingBox getTransformedBoundingBox(Transform::pointer transform) const;
        DataBoundingBox getTransformedBoundingBox(Affine3f transform) const;
        bool isInitialized() const;
    private:
        void initialize(const std::vector<Vector3f>& coords);
        void createCorners(Vector3f pos, Vector3f size);
        MatrixXf mCorners;
        bool mIsInitialized;

};


std::ostream &operator<<(std::ostream &os, DataBoundingBox &object);

} // end namespace fast

