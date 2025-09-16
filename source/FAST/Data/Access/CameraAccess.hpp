#pragma once

#include "FAST/Data/DataTypes.hpp"

namespace fast {

/**
 * @brief CPU access to Camera
 * @ingroup access
 */
class FAST_EXPORT  CameraAccess {
    public:
        CameraAccess(Vector3f* position, Vector3f* upVector, Vector3f* target);
        Affine3f getCameraTransformation() const;
        Vector3f getTargetPosition() const;
        Vector3f getPosition() const;
        Vector3f getUpVector() const;
        void setTargetPosition(Vector3f position);
        void setPosition(Vector3f position);
        void setUpVector(Vector3f upVector);
        typedef std::unique_ptr<CameraAccess> pointer;
    private:
        Vector3f* mPosition;
        Vector3f* mUpVector;
        Vector3f* mTarget;
};

}
