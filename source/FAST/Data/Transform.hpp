#pragma once

#include <FAST/Data/DataObject.hpp>
#include <FAST/Data/DataTypes.hpp>

namespace fast {

/**
 * @brief Data object for a 3D affine transform.
 *
 * The 3D transform is stored as an Eigen Affine3f object (4x4 32 bit float matrix)
 * @ingroup data
 */
class FAST_EXPORT Transform : public DataObject {
    FAST_DATA_OBJECT(Transform)
    public:
#ifndef SWIG
        FAST_CONSTRUCTOR(Transform, Affine3f, transform,);
        FAST_CONSTRUCTOR(Transform, Matrix4f, matrix,);
#endif
        FAST_CONSTRUCTOR(Transform,
                         Vector3f, translation, = Vector3f::Zero(),
                         Vector3f, scaling, = Vector3f::Ones(),
                         float, rotation, = 0.0f,
                         Vector3f, rotationAxis, = Vector3f::UnitX()
        );
        Affine3f get() const;
        void set(Affine3f transform);
        Matrix4f getMatrix() const;
        void setMatrix(Matrix4f matrix);
        void setTranslation(Vector3f translation);
        Vector3f getTranslation() const;
        void addTranslation(Vector3f translation);
        void setRotation(float angles, Vector3f axis);
        void addRotation(float angles, Vector3f axis);
        AngleAxisf getRotation() const;
        void setScaling(Vector3f scaling);
        void addScaling(Vector3f scaling);
        void free(ExecutionDevice::pointer device) {};
        void freeAll() {};
        ~Transform() {}
    private:
        Affine3f m_transform = Affine3f::Identity();

};

}