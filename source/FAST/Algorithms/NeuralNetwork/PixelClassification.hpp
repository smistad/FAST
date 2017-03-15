#ifndef FAST_PIXEL_CLASSIFICATION_HPP_
#define FAST_PIXEL_CLASSIFICATION_HPP_

#include "NeuralNetwork.hpp"

namespace fast {
class PixelClassification : public NeuralNetwork {
    FAST_OBJECT(PixelClassification)
    public:
        void setNrOfClasses(uint classes);
        void setHeatmapOutput();
        void setSegmentationOutput();
    private:
        PixelClassification();
        void execute();

        int mNrOfClasses;
        bool mHeatmapOutput;

};

}

#endif