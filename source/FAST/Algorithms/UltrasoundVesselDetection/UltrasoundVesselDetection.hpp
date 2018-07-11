#ifndef VESSEL_DETECTION_FOR_2D_ULTRASOUND_HPP
#define VESSEL_DETECTION_FOR_2D_ULTRASOUND_HPP

#include "FAST/ProcessObject.hpp"
#include "FAST/Algorithms/UltrasoundVesselDetection/VesselCrossSection.hpp"

namespace fast {

class ImageClassifier;

class FAST_EXPORT  UltrasoundVesselDetection : public ProcessObject {
    FAST_OBJECT(UltrasoundVesselDetection)
    public:
        ProcessObjectPort getOutputSegmentationPort();
        std::vector<VesselCrossSection::pointer> getCrossSections();
        std::vector<VesselCrossSection::pointer> getAcceptedCrossSections();
    private:
        UltrasoundVesselDetection();
        void execute();

        bool mCreateSegmentation;
        std::vector<VesselCrossSection::pointer> mCrossSections;
        std::vector<VesselCrossSection::pointer> mAcceptedCrossSections;
        SharedPointer<ImageClassifier> mClassifier;
};

}

#endif
