#include "TubeSegmentationAndCenterlineExtraction.hpp"
#include "FAST/Data/Image.hpp"
#include "FAST/Data/Segmentation.hpp"
#include "FAST/Data/LineSet.hpp"

namespace fast {

TubeSegmentationAndCenterlineExtraction::TubeSegmentationAndCenterlineExtraction() {

    createInputPort<Image>(0);
    createOutputPort<Segmentation>(0, OUTPUT_DEPENDS_ON_INPUT, 0);
    createOutputPort<LineSet>(1, OUTPUT_DEPENDS_ON_INPUT, 0);

    mSensitivity = 0.5;
    mMinimumRadius = 0.5;
    mMaximumRadius = 5;
    mRadiusStep = 0.5;
    mExtractDarkStructures = false;
    mSegmentation = true;
    mThresholdCropping = false;
    mLungCropping = false;
    mMinimumIntensity = std::numeric_limits<float>::min();
    mMaximumIntensity = std::numeric_limits<float>::max();
    mStDevBlurSmall = 0.5;
    mStDevBlurLarge = 0.5;
}

void TubeSegmentationAndCenterlineExtraction::loadPreset() {
}

void TubeSegmentationAndCenterlineExtraction::setMinimumRadius(
        float radius) {
    mMinimumRadius = radius;
}

void TubeSegmentationAndCenterlineExtraction::setMaximumRadius(
        float radius) {
    mMaximumRadius = radius;
}

void TubeSegmentationAndCenterlineExtraction::setRadiusStep(float step) {
    mRadiusStep = step;
}

void TubeSegmentationAndCenterlineExtraction::setSensitivity(
        float sensitivity) {
    mSensitivity = sensitivity;
}

void TubeSegmentationAndCenterlineExtraction::setMinimumIntensity(
        float intensity) {
    mMinimumIntensity = intensity;
}

void TubeSegmentationAndCenterlineExtraction::setMaximumIntensity(
        float intensity) {
    mMaximumIntensity = intensity;
}

void TubeSegmentationAndCenterlineExtraction::extractDarkTubes() {
    mExtractDarkStructures = true;
}

void TubeSegmentationAndCenterlineExtraction::extractBrightTubes() {
    mExtractDarkStructures = false;
}

void TubeSegmentationAndCenterlineExtraction::disableSegmentation() {
    mSegmentation = false;
}

void TubeSegmentationAndCenterlineExtraction::enableSegmentation() {
    mSegmentation = true;
}

void TubeSegmentationAndCenterlineExtraction::disableAutomaticCropping() {
    mThresholdCropping = false;
    mLungCropping = false;
}

void TubeSegmentationAndCenterlineExtraction::enableAutomaticCropping(
        bool lungCropping) {
    if(lungCropping) {
        mLungCropping = true;
    } else {
        mThresholdCropping = true;
    }
}

ProcessObjectPort TubeSegmentationAndCenterlineExtraction::getSegmentationOutputPort() {
    return getOutputPort(0);
}

ProcessObjectPort TubeSegmentationAndCenterlineExtraction::getCenterlineOutputPort() {
    return getOutputPort(1);
}


void TubeSegmentationAndCenterlineExtraction::execute() {
    Image::pointer input = getStaticInputData<Image>();
    Vector3f spacing = input->getSpacing();
    float smallestSpacing = spacing.minCoeff();
    float largestSpacing = spacing.maxCoeff();

    // Cropping

    // If min radius is larger than 2.5 voxels
    bool smallTDFExecuted = false;
    if(mMinimumRadius / smallestSpacing < 2.5) {
        // Find small structures
        // Blur
        // Create gradients and cap intensity
        // TDF
        smallTDFExecuted = true;
    }

    // If max radius is larger than 2.5 voxels
    bool largeTDFExecuted = false;
    if(mMaximumRadius / largestSpacing > 2.5) {
        // Find large structures, if max radius is large enough
        // Blur
        // Create gradients and cap intensity
        // GVF
        // TDF
        largeTDFExecuted = true;
    }

    // Add the two TDF's if applicable
    if(smallTDFExecuted && largeTDFExecuted) {

    }

    // Centerline extraction

    // Segmentation
}

} // end namespace fast
