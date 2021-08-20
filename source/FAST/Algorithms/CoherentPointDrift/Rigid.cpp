#include "CoherentPointDrift.hpp"
#include "Rigid.hpp"

#include <limits>
#include <iostream>

namespace fast {

    CoherentPointDriftRigid::CoherentPointDriftRigid() {
        mScale = 1.0;
        mTransformationType = TransformationType::RIGID;
    }

    void CoherentPointDriftRigid::initializeVarianceAndMore() {

        // Initialize the variance in the CPD registration
        mVariance = ((double) mNumMovingPoints * (mFixedPoints.transpose() * mFixedPoints).trace() +
                     (double) mNumFixedPoints * (mMovingPoints.transpose() * mMovingPoints).trace() -
                     2.0 * mFixedPoints.colwise().sum() * mMovingPoints.colwise().sum().transpose()) /
                    (double) (mNumFixedPoints * mNumMovingPoints * mNumDimensions);

        mIterationError = mTolerance + 10.0;
        mObjectiveFunction = std::numeric_limits<double>::max();
        mResponsibilityMatrix = MatrixXf::Zero(mNumMovingPoints, mNumFixedPoints);
        mPt1 = VectorXf::Zero(mNumFixedPoints);
        mP1 = VectorXf::Zero(mNumMovingPoints);
    }

    void CoherentPointDriftRigid::maximization(MatrixXf& fixedPoints, MatrixXf& movingPoints) {
        double startM = omp_get_wtime();

        // Calculate some useful matrix reductions
        mP1 = VectorXf::Zero(mNumMovingPoints);
        #pragma omp parallel for
            for (int col = 0; col < mNumFixedPoints; ++col) {
                mPt1(col) = mResponsibilityMatrix.col(col).sum();
            }
        #pragma omp parallel
        {
            VectorXf mP1Local = VectorXf::Zero(mNumMovingPoints);
            #pragma omp for
            for (int col = 0; col < mNumFixedPoints; ++col) {
                mP1Local += mResponsibilityMatrix.col(col);
            }
            #pragma omp critical
            mP1 += mP1Local;
        }
        mNp = mPt1.sum();                                           // 1 (sum of all P elements)
        double timeEndMUseful = omp_get_wtime();

        // Estimate new mean vectors
        MatrixXf fixedMean = fixedPoints.transpose() * mPt1 / mNp;
        MatrixXf movingMean = movingPoints.transpose() * mP1 / mNp;

        // Center point sets around estimated mean
        MatrixXf fixedPointsCentered = fixedPoints - fixedMean.transpose().replicate(mNumFixedPoints, 1);
        MatrixXf movingPointsCentered = movingPoints - movingMean.transpose().replicate(mNumMovingPoints, 1);

        double timeEndMCenter = omp_get_wtime();


        // Single value decomposition (SVD)
        const MatrixXf A = fixedPointsCentered.transpose() * mResponsibilityMatrix.transpose() * movingPointsCentered;
        auto svdU =  A.bdcSvd(Eigen::ComputeThinU);
        auto svdV =  A.bdcSvd(Eigen::ComputeThinV);
        const MatrixXf* U = &svdU.matrixU();
        const MatrixXf* V = &svdV.matrixV();

        MatrixXf UVt = *U * V->transpose();
        Eigen::RowVectorXf C = Eigen::RowVectorXf::Ones(mNumDimensions);
        C[mNumDimensions-1] = UVt.determinant();

        double timeEndMSVD = omp_get_wtime();

        /* ************************************************************
         * Find transformation parameters: rotation, scale, translation
         * ***********************************************************/
        mRotation = *U * C.asDiagonal() * V->transpose();
        MatrixXf AtR = A.transpose() * mRotation;
        MatrixXf ARt = A * mRotation.transpose();
        double traceAtR = AtR.trace();
        double traceXPX = (fixedPointsCentered.transpose() * mPt1.asDiagonal() * fixedPointsCentered).trace();
        double traceYPY = (movingPointsCentered.transpose() * mP1.asDiagonal() * movingPointsCentered).trace();

        mScale = traceAtR / traceYPY;
        mTranslation = fixedMean - mScale * mRotation * movingMean;

        // Update variance
        mVariance = ( traceXPX - mScale * traceAtR ) / (mNp * mNumDimensions);
        if (mVariance < 0) {
            mVariance = std::fabs(mVariance);
        } else if (mVariance == 0){
            mVariance = 10.0 * std::numeric_limits<double>::epsilon();
            mRegistrationConverged = true;
        }
        double timeEndMParameters = omp_get_wtime();

        /* ****************
         * Update transform
         * ***************/
        Affine3f iterationTransform = Affine3f::Identity();
        iterationTransform.translation() = Vector3f(mTranslation);
        iterationTransform.linear() = mRotation;
        iterationTransform.scale(float(mScale));


        Affine3f currentRegistrationTransform;
        MatrixXf registrationMatrix = iterationTransform.matrix() * mTransformation->get().matrix();
        currentRegistrationTransform.matrix() = registrationMatrix;
        mTransformation->set(currentRegistrationTransform);


        /* *************************
         * Transform the point cloud
         * ************************/
        MatrixXf movingPointsTransformed =
                mScale * movingPoints * mRotation.transpose() + mTranslation.transpose().replicate(mNumMovingPoints, 1);
        movingPoints = movingPointsTransformed;


        /* ******************************************
         * Calculate change in the objective function
         * *****************************************/
        double objectiveFunctionOld = mObjectiveFunction;
        mObjectiveFunction =
                (traceXPX - 2 * mScale * ARt.trace() + mScale * mScale * traceYPY) / (2 * mVariance)
                + (mNp * mNumDimensions)/2 * log(mVariance);
        mIterationError = std::fabs( (mObjectiveFunction - objectiveFunctionOld) / objectiveFunctionOld);
        mRegistrationConverged =  mIterationError <= mTolerance;


        double endM = omp_get_wtime();
        timeM += endM - startM;
        timeMUseful += timeEndMUseful - startM;
        timeMCenter += timeEndMCenter - timeEndMUseful;
        timeMSVD += timeEndMSVD - timeEndMCenter;
        timeMParameters += timeEndMParameters - timeEndMSVD;
        timeMUpdate += endM - timeEndMParameters;
    }


}