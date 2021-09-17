#include "Transform.hpp"

namespace fast {

Transform::Transform(Matrix4f matrix) {
    m_transform.matrix() = matrix;
}

Transform::Transform(Affine3f transform) {
    m_transform = transform;
}

Transform::Transform(Vector3f translation, Vector3f scaling, float rotation, Vector3f rotationAxis) {
    if(translation != Vector3f::Zero())
        setTranslation(translation);
    if(scaling != Vector3f::Ones())
        setScaling(scaling);
    if(rotation > 0)
        setRotation(rotation, rotationAxis);
}

Affine3f Transform::get() const {
    return m_transform;
}

void Transform::set(Affine3f transform) {
    m_transform = transform;
}

Matrix4f Transform::getMatrix() const {
    return m_transform.matrix();
}

void Transform::setMatrix(Matrix4f matrix) {
    m_transform.matrix() = matrix;
}

void Transform::setTranslation(Vector3f translation) {
    m_transform.translation() = translation;
}

Vector3f Transform::getTranslation() const {
    return m_transform.translation();
}

void Transform::addTranslation(Vector3f translation) {
    m_transform.translate(translation);
}

void Transform::setRotation(float angles, Vector3f axis) {
    m_transform.linear() = Matrix3f::Identity(); // Clear existing rotation and scaling
    m_transform.rotate(AngleAxisf(angles, axis));
}

void Transform::addRotation(float angles, Vector3f axis) {
    m_transform.rotate(AngleAxisf(angles, axis));
}

AngleAxisf Transform::getRotation() const {
    return AngleAxisf(m_transform.rotation());
}

void Transform::setScaling(Vector3f scaling) {
    m_transform.linear() = Matrix3f::Identity(); // Clear existing rotation and scaling
    m_transform.scale(scaling);
}

void Transform::addScaling(Vector3f scaling) {
    m_transform.scale(scaling);
}


}
