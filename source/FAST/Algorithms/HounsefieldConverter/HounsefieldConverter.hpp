#ifndef FAST_HOUNSEFIELD_CONVERTER_HPP_
#define FAST_HOUNSEFIELD_CONVERTER_HPP_

#include "FAST/ProcessObject.hpp"

namespace fast {

class Image;

class HounsefieldConverter : public ProcessObject {
    FAST_OBJECT(HounsefieldConverter)
    public:
    private:
        HounsefieldConverter();
        void execute();
        SharedPointer<Image> convertToHU(SharedPointer<Image> image);
};

}


#endif
