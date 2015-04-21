#ifndef PLANE_HPP_
#define PLANE_HPP_

#include "DataTypes.hpp"

namespace fast {

class Plane {
    public:
        Plane(Vector3f normal);
        Plane(Vector3f normal, Vector3f position);
        void setPosition(Vector3f position);
        void setNormal(Vector3f normal);
        Vector3f getPosition() const;
        Vector3f getNormal() const;
    private:
        static Plane Sagittal();
        static Plane Coronal();
        static Plane Axial();

        bool mHasPosition;
        Vector3f mNormal;
        Vector3f mPosition;
};

}

#endif
