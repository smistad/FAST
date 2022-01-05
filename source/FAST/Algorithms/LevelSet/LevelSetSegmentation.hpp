#pragma once

#include "FAST/ProcessObject.hpp"

namespace fast {

/**
 * @brief Level set image segmentation
 *
 * GPU-based level set segmentation using spherical seed points.
 * Only supports 3D images atm.
 *
 * Inputs:
 * - 0: Image 3D
 *
 * Outputs:
 * - 0: Image segmentation 3D
 *
 * @ingroup segmentation
 */
class FAST_EXPORT  LevelSetSegmentation : public ProcessObject {
    FAST_PROCESS_OBJECT(LevelSetSegmentation)
    public:
        /**
         * @brief Create instance
         * @param seedPoints List of 3D points to create seeds
         * @param seedRadius Radius of seeds
         * @param curvatureWeight Weight for curvature term
         * @param maxIterations Maximum number of iterations
         * @return instance
         */
        FAST_CONSTRUCTOR(LevelSetSegmentation,
                     std::vector<Vector3i>, seedPoints,,
                     float, seedRadius, = 1.0f,
                     float, curvatureWeight, = 0.9f,
                     int, maxIterations, = 1000
         )
        void addSeedPoint(Vector3i position, float radius);
        void setCurvatureWeight(float weight);
        void setIntensityMean(float intensity);
        void setIntensityVariance(float variation);
        void setMaxIterations(uint iterations);
    private:
        LevelSetSegmentation();
        void execute();

        std::vector<std::pair<Vector3i, float> > mSeeds;

        float mCurvatureWeight, mIntensityMean, mIntensityVariance;
        bool mIntensityMeanSet;
        bool mIntensityVarianceSet;
        int mIterations;

};

}