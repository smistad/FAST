#pragma once

#include "FAST/ProcessObject.hpp"
#include "FAST/Data/Mesh.hpp"
#include "FAST/Algorithms/CoherentPointDrift/CoherentPointDrift.hpp"

namespace fast {

    class FAST_EXPORT CoherentPointDriftAffine: public CoherentPointDrift {
    FAST_OBJECT(CoherentPointDriftAffine);
    public:
        CoherentPointDriftAffine();
        void initializeVarianceAndMore() override;
        void maximization(MatrixXf& fixedPoints, MatrixXf& movingPoints) override;

    private:
        MatrixXf mPt1;                          // Colwise sum of P, then transpose
        MatrixXf mP1;                           // Rowwise sum of P
        MatrixXf mAffineMatrix;                 // B
        MatrixXf mTranslation;                  // t
        double mIterationError;                 // Change in error from iteration to iteration
        float mNp;                              // Sum of all elements in P
        TransformationType mTransformationType;
    };

}

