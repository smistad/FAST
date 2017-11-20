#ifndef FAST_ULTRASOUND_IMAGE_ENHANCEMENT_HPP_
#define FAST_ULTRASOUND_IMAGE_ENHANCEMENT_HPP_

#include <FAST/ProcessObject.hpp>

namespace fast {

class UltrasoundImageEnhancement : public ProcessObject {
    FAST_OBJECT(UltrasoundImageEnhancement)
    public:
        void loadAttributes();
        void setReject(int value);
    private:
        UltrasoundImageEnhancement();
        void execute();

        std::vector<uchar> mColormap;
        cl::Buffer mColormapBuffer;
        bool mColormapUploaded;

};

}

#endif