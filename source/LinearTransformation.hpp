#ifndef LINEARTRANSFORMATION_HPP_
#define LINEARTRANSFORMATION_HPP_

#include "DataTypes.hpp"

namespace fast {

class LinearTransformation {
    public:
        LinearTransformation();
        LinearTransformation getInverse();
        LinearTransformation operator*(LinearTransformation other);
        void setTransform(const Eigen::Transform<float, 3, Eigen::Affine> transform);
        Eigen::Transform<float, 3, Eigen::Affine> getTransform() const;
        Vector3f getEulerAngles() const;
        Vector3f operator*(Vector3f vertex) const;
        float& operator()(uint i, uint j);
        ~LinearTransformation() {};
    private:
        Eigen::Transform<float, 3, Eigen::Affine> mTransform;
        void free(ExecutionDevice::pointer device) {};
        void freeAll() {};
};

} // end namespace fast

#endif /* LINEARTRANSFORMATION_HPP_ */
