#ifndef IMAGEACCESS_HPP_
#define IMAGEACCESS_HPP_

#include "SmartPointers.hpp"
#include "DataTypes.hpp"

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
    private:
        void* mData;
        bool* mAccessFlag;
        bool* mAccessFlag2;

        SharedPointer<Image> mImage;
};

} // end namespace fast



#endif /* IMAGEACCESS2D_HPP_ */
