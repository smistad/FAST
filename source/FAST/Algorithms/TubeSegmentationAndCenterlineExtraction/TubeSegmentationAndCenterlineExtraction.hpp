#ifndef TUBE_SEGMENTATION_AND_CENTERLINE_EXTRACTION_HPP
#define TUBE_SEGMENTATION_AND_CENTERLINE_EXTRACTION_HPP

#include "ProcessObject.hpp"

namespace fast {

class TubeSegmentationAndCenterlineExtraction : public ProcessObject {
    FAST_OBJECT(TubeSegmentationAndCenterlineExtraction)
    public:
        void loadPreset();
        void setMinimumRadius(float radius);
        void setMaximumRadius(float radius);
        void setRadiusStep(float step);
        void setSensitivity(float sensitivity);
        // The voxel intensities are capped to these values
        void setMinimumIntensity(float intensity);
        // The voxel intensities are capped to these values
        void setMaximumIntensity(float intensity);
        void extractDarkTubes();
        void extractBrightTubes();
        void disableSegmentation();
        void enableSegmentation();
        // TODO move cropping out of this algorithm
        void disableAutomaticCropping();
        void enableAutomaticCropping(bool lungCropping = false);
        ProcessObjectPort getSegmentationOutputPort();
        ProcessObjectPort getCenterlineOutputPort();
    private:
        TubeSegmentationAndCenterlineExtraction();
        void execute();

        // Parameters

        // General
        bool mSegmentation, mThresholdCropping, mLungCropping;
        float mStDevBlurSmall, mStDevBlurLarge; // This should be tuned to the amount of noise in the image.
        float mMinimumIntensity, mMaximumIntensity; // The voxel intensities are capped to these values
        bool mExtractDarkStructures; // true and this extract dark structures, false and it extract bright structures

        // Radius
        float mMinimumRadius, mMaximumRadius, mRadiusStep;

        // Centerline extraction
        float mSensitivity; // A number between 0 and 1 influencing how much is extracted (including noise).
};

} // end namespace fast

#endif
