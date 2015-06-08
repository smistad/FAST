#ifndef SEGMENTATION_HPP_
#define SEGMENTATION_HPP_

#include "Image.hpp"
#include "Color.hpp"

namespace fast {

// TODO inheritance here should probably not be public..
class Segmentation : public Image {
    FAST_OBJECT(Segmentation)
    public:
        void createFromImage(Image::pointer image, ExecutionDevice::pointer device);

        // If you add a label to this enum you should also add a default color in the SegmentationRenderer constructor
        enum LabelType {
            LABEL_BACKGROUND = 0,
            LABEL_FOREGROUND,
            LABEL_BLOOD,
            LABEL_BONE,
            LABEL_MUSCLE
        };

        /*
        using Image::getSpacing;
        using Image::getWidth;
        using Image::getHeight;
        using Image::getDepth;
        */
        ~Segmentation();
    protected:
        Segmentation();

};

}

#endif
