#pragma once

#include "FAST/Data/DataTypes.hpp"

namespace fast {

/**
 * @brief Class for a 3D plane
 *
 * A plane is defined by a 3D position and a 3D normal
 */
class FAST_EXPORT  Plane {
    public:
        Plane();
        Plane(Vector3f normal);
        Plane(Vector3f normal, Vector3f position);
        void setPosition(Vector3f position);
        void setNormal(Vector3f normal);
        Vector3f getPosition() const;
        Vector3f getNormal() const;
        Vector3f getIntersectionPoint(Vector3f pointA, Vector3f pointB) const;
        bool hasPosition() const;
        static Plane Sagittal();
        static Plane Coronal();
        static Plane Axial();
    private:

        bool mHasPosition;
        Vector3f mNormal;
        Vector3f mPosition;
};

}
