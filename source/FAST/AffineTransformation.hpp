#ifndef AFFINE_TRANSFORMATION_HPP_
#define AFFINE_TRANSFORMATION_HPP_

#include "FAST/Data/DataObject.hpp"
#include "FAST/Data/DataTypes.hpp"

namespace fast {

class AffineTransformation : public DataObject, public Eigen::Affine3f {
    FAST_OBJECT(AffineTransformation);
    public:
        AffineTransformation();
        AffineTransformation(const Eigen::Affine3f& transform);
        AffineTransformation::pointer multiply(AffineTransformation::pointer);
        Vector3f multiply(Vector3f);
        AffineTransformation& operator=(const Eigen::Affine3f& transform);
        Vector3f getEulerAngles() const;
        ~AffineTransformation() {};
        void free(ExecutionDevice::pointer device) {};
        void freeAll() {};
    private:
};

} // end namespace fast

#endif /* AFFINE_TRANSFORMATION_HPP_ */
