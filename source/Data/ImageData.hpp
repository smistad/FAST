#ifndef IMAGEDATA_HPP_
#define IMAGEDATA_HPP_

#include "DataObject.hpp"

namespace fast {

class ImageData : public DataObject {
    public:
        typedef SharedPointer<ImageData> pointer;
        virtual ~ImageData() {};
        bool isDynamicData();
    protected:
        bool mIsDynamicData;

};

}




#endif /* IMAGEDATA_HPP_ */
