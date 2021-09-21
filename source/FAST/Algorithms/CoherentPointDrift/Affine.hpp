#pragma once

#include "FAST/ProcessObject.hpp"
#include "FAST/Data/Mesh.hpp"
#include "FAST/Algorithms/CoherentPointDrift/CoherentPointDrift.hpp"

namespace fast {

/**
 * @brief Affine coherent point drift registration
 *
 * @ingroup registration
*/
class FAST_EXPORT CoherentPointDriftAffine: public CoherentPointDrift {
    FAST_PROCESS_OBJECT(CoherentPointDriftAffine);
    public:
        FAST_CONSTRUCTOR(CoherentPointDriftAffine);
        FAST_CONNECT(CoherentPointDriftAffine, Fixed, 0);
        FAST_CONNECT(CoherentPointDriftAffine, Moving, 1);
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

