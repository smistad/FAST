#pragma once

#include "FAST/ProcessObject.hpp"

namespace fast {

/**
 * @brief Extract triangle mesh from a 3D Image (volume)
 *
 * This process object uses the Marching Cubes algorithm to extract a isosurface, a triangle Mesh,
 * from a 3D Image. This GPU OpenCL implementation uses histogram pyramids and is documented in the article
 * "Real-time surface extraction and visualization of medical images using OpenCL and GPUs" Smistad et al. 2012
 *
 * Inputs:
 * - 0: Image 3D
 *
 * Outputs:
 * - 0: Mesh - Surface triangle mesh extracted from input image
 *
 * @ingroup segmentation
 */
class FAST_EXPORT  SurfaceExtraction : public ProcessObject {
    FAST_PROCESS_OBJECT(SurfaceExtraction)
    public:
        /**
         * @brief Create instance
         * @param threshold Intensity threshold to accept a voxel as part the segmentation.
         * @return instance
         */
        FAST_CONSTRUCTOR(SurfaceExtraction, float, threshold, = 0.0f)
        void setThreshold(float threshold);
        float getThreshold() const;
    private:
        void execute();

        float mThreshold;
        unsigned int mHPSize;
        cl::Program program;
        // HP
        std::vector<cl::Image3D> images;
        std::vector<cl::Buffer> buffers;

        cl::Buffer cubeIndexesBuffer;
};

} // end namespace fast
