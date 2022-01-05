#pragma once

#include "FAST/ProcessObject.hpp"
#include "FAST/Data/Image.hpp"
#include "FAST/Data/DataTypes.hpp"

namespace fast {

/**
 * @brief Segmentation by seeded region growing
 *
 * Inputs:
 * - 0: Image
 *
 * Outputs
 * - 0: Image segmentation
 *
 * @ingroup segmentation
 */
class FAST_EXPORT SeededRegionGrowing : public ProcessObject {
    FAST_PROCESS_OBJECT(SeededRegionGrowing)
    public:
        /**
         * @brief Create instance
         * @param intensityMinimum Minimum intensity to accept voxel as part of segmentation.
         * @param intensityMaximum Maximum intensity to accept voxel as part of segmentation.
         * @param seedPoints List of seed points in pixel space. If 2D, z component is not used.
         * @return instance
         */
        FAST_CONSTRUCTOR(SeededRegionGrowing,
                         float, intensityMinimum,,
                         float, intensityMaximum,,
                         std::vector<Vector3i>, seedPoints,
        )
        void setIntensityRange(float min, float max);
        void addSeedPoint(uint x, uint y);
        void addSeedPoint(uint x, uint y, uint z);
        void addSeedPoint(Vector3i position);
    private:
        SeededRegionGrowing();
        void execute();
        void waitToFinish();
        void recompileOpenCLCode(Image::pointer input);
        template <class T>
        void executeOnHost(T* input, Image::pointer output);

        float mMinimumIntensity, mMaximumIntensity;
        std::vector<Vector3i> mSeedPoints;

        cl::Kernel mKernel;
        unsigned char mDimensionCLCodeCompiledFor;
        DataType mTypeCLCodeCompiledFor;

};

} // end namespace fast
