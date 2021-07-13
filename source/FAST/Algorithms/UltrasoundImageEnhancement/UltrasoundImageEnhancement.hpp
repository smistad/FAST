#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

/**
 * @brief Apply a color map and reject on an ultrasound image
 *
 * Inputs:
 * - 0: Image grayscale
 *
 * Outputs:
 * - 0: Image color
 *
 * @ingroup ultrasound
 */
class FAST_EXPORT UltrasoundImageEnhancement : public ProcessObject {
    FAST_PROCESS_OBJECT(UltrasoundImageEnhancement)
    public:
        FAST_CONSTRUCTOR(UltrasoundImageEnhancement, int, reject, = 40);
        void loadAttributes();
        void setReject(int value);
    private:
        void execute();

        std::vector<uchar> mColormap;
        cl::Buffer mColormapBuffer;
        bool mColormapUploaded;

};

}