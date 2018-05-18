#ifndef FAST_PIXEL_CLASSIFIER_HPP_
#define FAST_PIXEL_CLASSIFIER_HPP_

#include "NeuralNetwork.hpp"

namespace fast {
class FAST_EXPORT  PixelClassifier : public NeuralNetwork {
    FAST_OBJECT(PixelClassifier)
    public:
        void setNrOfClasses(uint classes);
        void setHeatmapOutput();
        void setSegmentationOutput();
        void setResizeBackToOriginalSize(bool resize);
        void setThreshold(float threshold);
        void loadAttributes();
    private:
        PixelClassifier();
        void execute();


        int mNrOfClasses;
        bool mHeatmapOutput;
        bool mResizeBackToOriginalSize;
        float mThreshold;

};

}

#endif