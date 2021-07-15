#pragma once

#include "FAST/ProcessObject.hpp"
#include "FAST/Data/Image.hpp"
#include "FAST/Data/Mesh.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

/**
 * @brief Segmentation and centerline extraction of tubular structures
 *
 * This implementation is described in the article
 * "GPU Accelerated Segmentation and Centerline Extraction of TubularStructures from Medical Images"
 * by Smistad et al. 2014
 * https://www.eriksmistad.no/wp-content/uploads/gpu_accelerated_extraction_of_tubular_structures.pdf
 *
 * Inputs:
 * - 0: Image 3D
 *
 * Outputs:
 * - 0: Image segmentation 3D
 * - 1: Mesh centerline
 * - 2: Image 3D tubular detection heatmap
 *
 * @ingroup segmentation
 */
class FAST_EXPORT TubeSegmentationAndCenterlineExtraction : public ProcessObject {
    FAST_PROCESS_OBJECT(TubeSegmentationAndCenterlineExtraction)
    public:
        FAST_CONSTRUCTOR(TubeSegmentationAndCenterlineExtraction,
            float, sensitivity, = 0.5f,
            float, minimumRadius, = 1.0f,
            float, maximumRadius, = 5.0f,
            float, radiusStep, = 0.5f,
            bool, extractBrightTubes, = false,
            bool, keepLargestTreeOnly, = false
        )
        void setKeepLargestTree(bool keep);
        void setMinimumTreeSize(int nrOfVoxels);
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
        DataChannel::pointer getSegmentationOutputPort();
        DataChannel::pointer getCenterlineOutputPort();
        DataChannel::pointer getTDFOutputPort();
    private:
        void execute();

        Image::pointer createGradients(Image::pointer image);
        void runTubeDetectionFilter(Image::pointer gradients, float minimumRadius, float maximumRadius, Image::pointer& TDF, Image::pointer& radius);
        void runNonCircularTubeDetectionFilter(Image::pointer gradients, float minimumRadius, float maximumRadius, Image::pointer& TDF, Image::pointer& radius);
        Image::pointer runGradientVectorFlow(Image::pointer vectorField);
        void keepLargestObjects(Image::pointer segmentation, Mesh::pointer& centerlines);

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

        bool mOnlyKeepLargestTree;
        int mMinimumTreeSize;
};

} // end namespace fast
