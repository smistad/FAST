#ifndef IMAGE_DATA_HPP
#define IMAGE_DATA_HPP

#include "ImageData.hpp"
#include "DataTypes.hpp"
namespace fast {

class StaticImage : public ImageData {
    public:
        typedef SharedPointer<StaticImage> pointer;
        unsigned int getWidth() const;
        unsigned int getHeight() const;
        unsigned char getDimensions() const;
        DataType getDataType() const;
        unsigned int getNrOfComponents() const;
        virtual ~StaticImage() {};
    protected:
        unsigned int mWidth, mHeight;
        unsigned char mDimensions;
        DataType mType;
        unsigned int mComponents;

};

} // end namespace fast

#endif
