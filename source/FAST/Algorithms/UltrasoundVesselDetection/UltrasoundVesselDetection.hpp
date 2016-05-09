#ifndef VESSEL_DETECTION_FOR_2D_ULTRASOUND_HPP
#define VESSEL_DETECTION_FOR_2D_ULTRASOUND_HPP

#include "FAST/ProcessObject.hpp"
#include "FAST/Algorithms/UltrasoundVesselDetection/VesselCrossSection.hpp"

namespace fast {

class ImageClassifier;

class UltrasoundVesselDetection : public ProcessObject {
    FAST_OBJECT(UltrasoundVesselDetection)
    public:
        void addInputConnection(ProcessObjectPort port);
        ProcessObjectPort getOutputImagePort();
        ProcessObjectPort getPointSetPort();
        float getDetectedRadius() const;
        float getDetectedFlattening() const;
        std::vector<VesselCrossSection::pointer> getCrossSections();
    private:
        UltrasoundVesselDetection();
        void execute();

        float mDetectedRadius;
        float mDetectedFlattening;
        bool mCreateSegmentation;
        std::vector<VesselCrossSection::pointer> mCrossSections;
        SharedPointer<ImageClassifier> mClassifier;
};

}

#endif
