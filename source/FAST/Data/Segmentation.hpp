#pragma once

#include "Image.hpp"
#include "Color.hpp"

namespace fast {

/**
 * @brief Segmentation image data object
 *
 * A segmentation data object is an @ref Image with a single channel and 8 bit unsigned integer data type.
 *
 * @ingroup data
 */
class FAST_EXPORT  Segmentation : public Image {
    FAST_OBJECT(Segmentation)
    public:
        void createFromImage(Image::pointer image);

        // If you add a label to this enum you should also add a default color in the SegmentationRenderer constructor
        enum LabelType {
            LABEL_BACKGROUND = 0,
            LABEL_FOREGROUND,
            LABEL_BLOOD,
            LABEL_VEIN,
            LABEL_ARTERY,
            LABEL_BONE,
            LABEL_MUSCLE,
            LABEL_NERVE,
            LABEL_YELLOW,
            LABEL_GREEN,
            LABEL_MAGENTA,
            LABEL_RED,
            LABEL_WHITE,
			LABEL_BLUE
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
