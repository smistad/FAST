#ifndef FAST_ULTRASOUND_IMAGE_ENHANCEMENT_HPP_
#define FAST_ULTRASOUND_IMAGE_ENHANCEMENT_HPP_

#include <FAST/ProcessObject.hpp>

namespace fast {

class UltrasoundImageEnhancement : public ProcessObject {
    FAST_OBJECT(UltrasoundImageEnhancement)
    public:
    private:
        UltrasoundImageEnhancement();
        void execute();

        std::vector<uchar> mColormap;
};

}

#endif