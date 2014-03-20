#ifndef IMAGE_DATA_HPP
#define IMAGE_DATA_HPP

#include "PipelineObject.hpp"
#include "DataTypes.hpp"
namespace fast {

class ImageData : public PipelineObject {
    protected:
        unsigned int mWidth, mHeight;
        DataType mType;

};

}; // end namespace fast

#endif
