#ifndef AFFINE_TRANSFORMATION_HPP_
#define AFFINE_TRANSFORMATION_HPP_

#include "FAST/Data/DataObject.hpp"
#include "FAST/Data/DataTypes.hpp"

namespace fast {

class FAST_EXPORT  AffineTransformation : public DataObject {
    FAST_OBJECT(AffineTransformation);
    public:
        AffineTransformation();
        AffineTransformation(const Affine3f& transform);
        AffineTransformation::pointer multiply(AffineTransformation::pointer);
        Vector3f multiply(Vector3f);
        AffineTransformation& operator=(const Affine3f& transform);
        Vector3f getEulerAngles() const;
        ~AffineTransformation() {};
        Affine3f& getTransform();
		void setTransform(Affine3f transform);
        void free(ExecutionDevice::pointer device) {};
        void freeAll() {};
    private:
        Affine3f mTransform;
};

} // end namespace fast

#endif /* AFFINE_TRANSFORMATION_HPP_ */
