#ifndef IMAGE_DATA_HPP
#define IMAGE_DATA_HPP

#include "DataObject.hpp"
#include "DataTypes.hpp"
namespace fast {

class ImageData : public DataObject {
    public:
        unsigned int getWidth() const;
        unsigned int getHeight() const;
        unsigned char getDimensions() const;
    protected:
        unsigned int mWidth, mHeight;
        unsigned char mDimensions;
        DataType mType;
        virtual ~ImageData() {};

};

}; // end namespace fast

#endif
