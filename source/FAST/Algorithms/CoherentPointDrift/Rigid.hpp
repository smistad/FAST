#pragma once

#include "FAST/ProcessObject.hpp"
#include "FAST/Data/Mesh.hpp"
#include "FAST/Algorithms/CoherentPointDrift/CoherentPointDrift.hpp"

namespace fast {

/**
 * @brief Rigid coherent point drift registration
 *
 * @ingroup registration
*/
class FAST_EXPORT CoherentPointDriftRigid: public CoherentPointDrift {
    FAST_PROCESS_OBJECT(CoherentPointDriftRigid);
    public:
        FAST_CONSTRUCTOR(CoherentPointDriftRigid);
        FAST_CONNECT(CoherentPointDriftRigid, Fixed, 0);
        FAST_CONNECT(CoherentPointDriftRigid, Moving, 1);
        void maximization(MatrixXf& fixedPoints, MatrixXf& movingPoints) override;
        void initializeVarianceAndMore() override;

    private:
        VectorXf mPt1;                          // Colwise sum of P, then transpose
        VectorXf mP1;                           // Rowwise sum of P
        MatrixXf mRotation;                     // R
        MatrixXf mTranslation;                  // t
        double mIterationError;                 // Change in error from iteration to iteration
        float mNp;                              // Sum of all elements in P
        TransformationType mTransformationType;
    };

}

