#ifndef CAMERA_HPP_
#define CAMERA_HPP_

#include "SpatialDataObject.hpp"
#include "FAST/Data/Access/Access.hpp"
#include "FAST/Data/Access/CameraAccess.hpp"

namespace fast {

class Camera : public DataObject {
    FAST_OBJECT(Camera)
    public:
        void create(Vector3f position, Vector3f upVector, Vector3f target);
        CameraAccess::pointer getAccess(accessType accessType);
        ~Camera() {};
    private:
        Camera();
        void freeAll();
        void free(ExecutionDevice::pointer device);

        Vector3f mPosition;
        Vector3f mUpVector;
        Vector3f mTarget;

};

} // end namespace fast

#endif
