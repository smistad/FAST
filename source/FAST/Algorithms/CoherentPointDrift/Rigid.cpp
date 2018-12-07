#include "CoherentPointDrift.hpp"
#include "Rigid.hpp"

#include <limits>
#include <iostream>

namespace fast {

    CoherentPointDriftRigid::CoherentPointDriftRigid() {
        mScale = 1.0;
        mIterationError = mTolerance + 1.0;
        mTransformationType = TransformationType::RIGID;
    }

    void CoherentPointDriftRigid::initializeVarianceAndMore() {

        // Initialize the variance in the CPD registration
        mVariance = ((double) mNumMovingPoints * (mFixedPoints.transpose() * mFixedPoints).trace() +
                     (double) mNumFixedPoints * (mMovingPoints.transpose() * mMovingPoints).trace() -
                     2.0 * mFixedPoints.colwise().sum() * mMovingPoints.colwise().sum().transpose()) /
                    (double) (mNumFixedPoints * mNumMovingPoints * mNumDimensions);

        mObjectiveFunction = -mIterationError - double(mNumFixedPoints * mNumDimensions)/2 * log(mVariance);
        mProbabilityMatrix = MatrixXf::Zero(mNumMovingPoints, mNumFixedPoints);
        mPt1 = VectorXf::Zero(mNumFixedPoints);
        mP1 = VectorXf::Zero(mNumMovingPoints);
    }


    void CoherentPointDriftRigid::expectation(MatrixXf& fixedPoints, MatrixXf& movingPoints) {

        double timeStartE = omp_get_wtime();

        /* **********************************************************************************
         * Calculate distances between the points in the two point sets
         * Let row i in P equal the squared distances from all fixed points to moving point i
         * *********************************************************************************/

        /* Implementation without OpenMP
        MatrixXf movingPointMatrix = MatrixXf::Zero(mNumMovingPoints, mNumFixedPoints);
        MatrixXf distances = MatrixXf::Zero(mNumMovingPoints, mNumFixedPoints);
        for (int i = 0; i < mNumMovingPoints; ++i) {
            movingPointMatrix = movingPoints.row(i).replicate(mNumFixedPoints, 1);
            distances = fixedPoints - movingPoints.row(i).replicate(mNumFixedPoints, 1);
            distances = fixedPoints - movingPointMatrix;            // Distance between all fixed points and moving point i
            distances = distances.cwiseAbs2();                            // Square distance components (3xN)
            mProbabilityMatrix.row(i) = distances.rowwise().sum();   // Sum x, y, z components (1xN)
        }
        */

        auto c = (float) (pow(2*(double)EIGEN_PI*mVariance, (double)mNumDimensions/2.0)
                   * (mUniformWeight/(1-mUniformWeight)) * (float)mNumMovingPoints/mNumFixedPoints);

        #pragma omp parallel for //collapse(2)
            for (int col = 0; col < mNumFixedPoints; ++col) {
                for (int row = 0; row < mNumMovingPoints; ++row) {
                    double norm = (fixedPoints.row(col) - movingPoints.row(row)).squaredNorm();
                    mProbabilityMatrix(row, col) = exp(norm / (-2.0 * mVariance));
                }
            }
        double timeEndFirstLoop = omp_get_wtime();

        #pragma omp parallel for
            for (int col = 0; col < mNumFixedPoints; ++col) {
                float denom = mProbabilityMatrix.col(col).sum() + c;
                mProbabilityMatrix.col(col) /= max(denom, Eigen::NumTraits<float>::epsilon() );
            }

        // Update computation times
        double timeEndE = omp_get_wtime();
        timeEDistances += 0.0;
        timeENormal += timeEndFirstLoop - timeStartE;
        timeEPosterior += 0.0;
        timeEPosteriorDivision += timeEndE - timeEndFirstLoop;
        timeE += timeEndE - timeStartE;
    }

    void CoherentPointDriftRigid::maximization(MatrixXf& fixedPoints, MatrixXf& movingPoints) {
        double startM = omp_get_wtime();

        // Calculate some useful matrix reductions
        mP1 = VectorXf::Zero(mNumMovingPoints);
        #pragma omp parallel for
            for (int col = 0; col < mNumFixedPoints; ++col) {
                mPt1(col) = mProbabilityMatrix.col(col).sum();
            }
        #pragma omp parallel
        {
            VectorXf mP1Local = VectorXf::Zero(mNumMovingPoints);
            #pragma omp for
            for (int col = 0; col < mNumFixedPoints; ++col) {
                mP1Local += mProbabilityMatrix.col(col);
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
        const MatrixXf A = fixedPointsCentered.transpose() * mProbabilityMatrix.transpose() * movingPointsCentered;
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
            mVariance = abs(mVariance);
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
        MatrixXf registrationMatrix = iterationTransform.matrix() * mTransformation->getTransform().matrix();
        currentRegistrationTransform.matrix() = registrationMatrix;
        mTransformation->setTransform(currentRegistrationTransform);


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
        mIterationError = abs( (mObjectiveFunction - objectiveFunctionOld) / objectiveFunctionOld);
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