#ifndef DYNAMICIMAGE_HPP_
#define DYNAMICIMAGE_HPP_

#include "ImageData.hpp"
#include "SmartPointers.hpp"

namespace fast {

class DynamicImage : public ImageData {
    public:
        typedef SharedPointer<DynamicImage> pointer;
        virtual ~DynamicImage() {};

};

}




#endif /* DYNAMICIMAGE_HPP_ */
