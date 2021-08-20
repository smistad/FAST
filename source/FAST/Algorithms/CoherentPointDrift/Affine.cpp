#include "CoherentPointDrift.hpp"
#include "Affine.hpp"

#include <limits>
#include <iostream>

namespace fast {

    CoherentPointDriftAffine::CoherentPointDriftAffine() {
        mScale = 1.0;
        mTransformationType = TransformationType::AFFINE;
    }

    void CoherentPointDriftAffine::initializeVarianceAndMore() {
        // Initialize the variance in the CPD registration
        mVariance = (   (double)mNumMovingPoints * (mFixedPoints.transpose() * mFixedPoints).trace() +
                        (double)mNumFixedPoints * (mMovingPoints.transpose() * mMovingPoints).trace() -
                        2.0 * mFixedPoints.colwise().sum() * mMovingPoints.colwise().sum().transpose()  ) /
                    (double)(mNumFixedPoints * mNumMovingPoints * mNumDimensions);

        mIterationError = mTolerance + 10.0;
        mObjectiveFunction = mObjectiveFunction = std::numeric_limits<double>::max();
        mResponsibilityMatrix = MatrixXf::Zero(mNumMovingPoints, mNumFixedPoints);
    }

    void CoherentPointDriftAffine::maximization(Eigen::MatrixXf &fixedPoints, Eigen::MatrixXf &movingPoints) {

        double startM = omp_get_wtime();

        // Define some useful matrix sums
        mPt1 = mResponsibilityMatrix.transpose().rowwise().sum();      // mNumFixedPoints x 1
        mP1 = mResponsibilityMatrix.rowwise().sum();                   // mNumMovingPoints x 1
        mNp = mPt1.sum();                                           // 1 (sum of all P elements)
        double timeEndMUseful = omp_get_wtime();

        // Estimate new mean vectors
        MatrixXf fixedMean = fixedPoints.transpose() * mPt1 / mNp;
        MatrixXf movingMean = movingPoints.transpose() * mP1 / mNp;

        // Center point sets around estimated mean
        MatrixXf fixedPointsCentered = fixedPoints - fixedMean.transpose().replicate(mNumFixedPoints, 1);
        MatrixXf movingPointsCentered = movingPoints - movingMean.transpose().replicate(mNumMovingPoints, 1);
        double timeEndMCenter = omp_get_wtime();

        /* **********************************************************
         * Find transformation parameters: affine matrix, translation
         * *********************************************************/
        MatrixXf A = fixedPointsCentered.transpose() * mResponsibilityMatrix.transpose() * movingPointsCentered;
        MatrixXf YPY = movingPointsCentered.transpose() * mP1.asDiagonal() * movingPointsCentered;
        MatrixXf XPX = fixedPointsCentered.transpose() * mPt1.asDiagonal() * fixedPointsCentered;

        mAffineMatrix = A * YPY.inverse();
        mTranslation = fixedMean - mAffineMatrix * movingMean;

        // Update variance
        MatrixXf ABt = A * mAffineMatrix.transpose();
        mVariance = ( XPX.trace() - ABt.trace() ) / (mNp * mNumDimensions);
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
        iterationTransform.linear() = mAffineMatrix;

        Affine3f currentRegistrationTransform;
        MatrixXf registrationMatrix = iterationTransform.matrix() * mTransformation->get().matrix();
        currentRegistrationTransform.matrix() = registrationMatrix;
        mTransformation->set(currentRegistrationTransform);


        /* *************************
         * Transform the point cloud
         * ************************/
        MatrixXf movingPointsTransformed =
                movingPoints * mAffineMatrix.transpose() + mTranslation.transpose().replicate(mNumMovingPoints, 1);
        movingPoints = movingPointsTransformed;


        /* ******************************************
         * Calculate change in the objective function
         * *****************************************/
        double objectiveFunctionOld = mObjectiveFunction;
        mObjectiveFunction =
                (XPX.trace() - 2 * ABt.trace() + YPY.trace() ) / (2 * mVariance)
                + (mNp * mNumDimensions)/2 * log(mVariance);
        mIterationError = std::fabs( (mObjectiveFunction - objectiveFunctionOld) / objectiveFunctionOld);
        mRegistrationConverged =  mIterationError <= mTolerance;

        double endM = omp_get_wtime();
        timeM += endM - startM;
        timeMUseful += timeEndMUseful - startM;
        timeMCenter += timeEndMCenter - timeEndMUseful;
        timeMParameters += timeEndMParameters - timeEndMCenter;
        timeMUpdate += endM - timeEndMParameters;
    }

}
