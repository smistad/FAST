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
        float getScalar(Vector2i position, uchar channel = 0) const;
        float getScalar(Vector3i position, uchar channel = 0) const;
        void setScalar(Vector2i position, float value, uchar channel = 0);
        void setScalar(Vector3i position, float value, uchar channel = 0);
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
