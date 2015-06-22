#ifndef CAMERA_ACCESS_HPP_
#define CAMERA_ACCESS_HPP_

#include "FAST/SmartPointers.hpp"
#include "FAST/Data/DataTypes.hpp"

namespace fast {

class CameraAccess {
    public:
        CameraAccess(Vector3f* position, Vector3f* upVector, Vector3f* target);
        Eigen::Affine3f getCameraTransformation() const;
        Vector3f getTargetPosition() const;
        Vector3f getPosition() const;
        Vector3f getUpVector() const;
        void setTargetPosition(Vector3f position);
        void setPosition(Vector3f position);
        void setUpVector(Vector3f upVector);
        typedef UniquePointer<CameraAccess> pointer;
    private:
        Vector3f* mPosition;
        Vector3f* mUpVector;
        Vector3f* mTarget;
};

}

#endif
