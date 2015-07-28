#ifndef IMAGEACCESS_HPP_
#define IMAGEACCESS_HPP_

#include "FAST/SmartPointers.hpp"
#include "FAST/Data/DataTypes.hpp"

namespace fast {

class Image;

class ImageAccess {
    public:
        ImageAccess(void* data, SharedPointer<Image> image, bool* accessFlag, bool* accessFlag2);
        void* get();
        float getScalar(VectorXi position, uchar channel = 0) const;
        Vector4f getVector(VectorXi position) const;
        void setScalar(VectorXi position, float value, uchar channel = 0);
        void setVector(VectorXi position, Vector4f value);
        void release();
        ~ImageAccess();
		typedef UniquePointer<ImageAccess> pointer;
    private:
		ImageAccess(const ImageAccess::pointer other);
		ImageAccess::pointer operator=(const ImageAccess::pointer other);
        void* mData;
        bool* mAccessFlag;
        bool* mAccessFlag2;

        SharedPointer<Image> mImage;
};

} // end namespace fast



#endif /* IMAGEACCESS2D_HPP_ */
