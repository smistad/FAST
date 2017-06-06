#ifndef FAST_LEVEL_SET_SEGMENTATION_HPP_
#define FAST_LEVEL_SET_SEGMENTATION_HPP_

#include "FAST/ProcessObject.hpp"

namespace fast {

class FAST_EXPORT  LevelSetSegmentation : public ProcessObject {
    FAST_OBJECT(LevelSetSegmentation)
    public:
        void addSeedPoint(Vector3i position, float size);
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

#endif