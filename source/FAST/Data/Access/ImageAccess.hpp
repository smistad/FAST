#ifndef IMAGEACCESS_HPP_
#define IMAGEACCESS_HPP_

#include "FAST/SmartPointers.hpp"
#include "FAST/Data/DataTypes.hpp"

namespace fast {

class Image;

class FAST_EXPORT  ImageAccess {
    public:
        ImageAccess(void* data, SharedPointer<Image> image);
        void* get();
        float getScalar(uint position, uchar channel = 0) const;
        float getScalar(VectorXi position, uchar channel = 0) const;
        Vector4f getVector(VectorXi position) const;
        void setScalar(uint position, float value, uchar channel = 0);
        void setScalar(VectorXi position, float value, uchar channel = 0);
		void setVector(uint position, Vector4f value);
        void setVector(VectorXi position, Vector4f value);
        void release();
        ~ImageAccess();
		typedef UniquePointer<ImageAccess> pointer;
    private:
		ImageAccess(const ImageAccess::pointer other);
		ImageAccess::pointer operator=(const ImageAccess::pointer other);
        void* mData;

        SharedPointer<Image> mImage;
};

} // end namespace fast



#endif /* IMAGEACCESS2D_HPP_ */
