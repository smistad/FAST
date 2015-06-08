#include "FAST/Algorithms/IterativeClosestPoint/IterativeClosestPoint.hpp"
#include "FAST/SceneGraph.hpp"
#undef min
#undef max
#include <limits>

namespace fast {

IterativeClosestPoint::IterativeClosestPoint() {
    createInputPort<PointSet>(0);
    createInputPort<PointSet>(1);
    mMaxIterations = 100;
    mMinErrorChange = 1e-5;
    mError = -1;
    mTransformationType = IterativeClosestPoint::RIGID;
    mIsModified = true;
}


void IterativeClosestPoint::setFixedPointSetPort(ProcessObjectPort port) {
    setInputConnection(0, port);
}
void IterativeClosestPoint::setMovingPointSetPort(ProcessObjectPort port) {
    setInputConnection(1, port);
}
void IterativeClosestPoint::setFixedPointSet(PointSet::pointer data) {
    setInputData(0, data);
}
void IterativeClosestPoint::setMovingPointSet(PointSet::pointer data) {
    setInputData(1, data);
}

AffineTransformation IterativeClosestPoint::getOutputTransformation() {
    return mTransformation;
}

float IterativeClosestPoint::getError() const {
    return mError;
}

/**
 * Create a new matrix which is matrix A rearranged.
 * This matrix has the same size as B
 */
inline MatrixXf rearrangeMatrixToClosestPoints(const MatrixXf A, const MatrixXf B) {
    MatrixXf result = MatrixXf::Constant(B.rows(), B.cols(), 0);

    // For each point in B, find the closest point in A
    for(uint b = 0; b < B.cols(); b++) {
        Vector3f pointInB = B.col(b);
        float minDistance = std::numeric_limits<float>::max();
        uint closestPoint = 0;
        for(uint a = 0; a < A.cols(); a++) {
            Vector3f pointInA = A.col(a);
            float distance = (pointInA-pointInB).norm();
            if(distance < minDistance) {
                minDistance = distance;
                closestPoint = a;
            }
        }
        result.col(b) = A.col(closestPoint);
    }

    return result;

}

/*
 * Get centroid
 */
Vector3f getCentroid(const MatrixXf m) {
    return m.rowwise().sum() / m.cols();
}

inline float sign(float value) {
    if(value > 0) {
        return 1.0f;
    } else if (value < 0) {
        return -1.0f;
    } else {
        return 0.0f;
    }
}

void IterativeClosestPoint::setTransformationType(
        const IterativeClosestPoint::TransformationType type) {
    mTransformationType = type;
    mIsModified = true;
}

void IterativeClosestPoint::execute() {
    float error = std::numeric_limits<float>::max(), previousError;
    uint iterations = 0;

    // Get access to the two point sets
    PointSetAccess::pointer accessFixedSet = ((PointSet::pointer)getStaticInputData<PointSet>(0))->getAccess(ACCESS_READ);
    PointSetAccess::pointer accessMovingSet = ((PointSet::pointer)getStaticInputData<PointSet>(1))->getAccess(ACCESS_READ);

    // Get transformations of point sets
    AffineTransformation fixedPointTransform = SceneGraph::getAffineTransformationFromData(getStaticInputData<PointSet>(0));
    AffineTransformation initialMovingTransform = SceneGraph::getAffineTransformationFromData(getStaticInputData<PointSet>(1));

    // These matrices are Nx3
    MatrixXf fixedPoints = accessFixedSet->getPointSetAsMatrix();
    MatrixXf movingPoints = accessMovingSet->getPointSetAsMatrix();

    Eigen::Transform<float, 3, Eigen::Affine> currentTransformation = Eigen::Transform<float, 3, Eigen::Affine>::Identity();

    // Want to choose the smallest one as moving
    bool invertTransform = false;
    if(false && fixedPoints.cols() < movingPoints.cols()) {
        std::cout << "switching fixed and moving" << std::endl;
        // Switch fixed and moving
        MatrixXf temp = fixedPoints;
        fixedPoints = movingPoints;
        movingPoints = temp;
        invertTransform = true;

        // Apply initial transformations
        //currentTransformation = fixedPointTransform.getTransform();
        movingPoints = fixedPointTransform*movingPoints.colwise().homogeneous();
        fixedPoints = initialMovingTransform*fixedPoints.colwise().homogeneous();
    } else {
        // Apply initial transformations
        //currentTransformation = initialMovingTransform.getTransform();
        movingPoints = initialMovingTransform*movingPoints.colwise().homogeneous();
        fixedPoints = fixedPointTransform*fixedPoints.colwise().homogeneous();
    }
    do {
        previousError = error;
        MatrixXf movedPoints = currentTransformation*(movingPoints.colwise().homogeneous());

        // Match closest points using current transformation
        MatrixXf rearrangedFixedPoints = rearrangeMatrixToClosestPoints(
                fixedPoints, movedPoints);

        // Get centroids
        Vector3f centroidFixed = getCentroid(rearrangedFixedPoints);
        //std::cout << "Centroid fixed: " << std::endl;
        //std::cout << centroidFixed << std::endl;
        Vector3f centroidMoving = getCentroid(movedPoints);
        //std::cout << "Centroid moving: " << std::endl;
        //std::cout << centroidMoving << std::endl;

        Eigen::Transform<float, 3, Eigen::Affine> updateTransform = Eigen::Transform<float, 3, Eigen::Affine>::Identity();

        if(mTransformationType == IterativeClosestPoint::RIGID) {
            // Create correlation matrix H of the deviations from centroid
            MatrixXf H = (movedPoints.colwise() - centroidMoving)*
                    (rearrangedFixedPoints.colwise() - centroidFixed).transpose();

            // Do SVD on H
            Eigen::JacobiSVD<Eigen::MatrixXf> svd(H, Eigen::ComputeFullU | Eigen::ComputeFullV);

            // Estimate rotation as R=V*U.transpose()
            MatrixXf temp = svd.matrixV()*svd.matrixU().transpose();
            Matrix3f d = Matrix3f::Identity();
            d(2,2) = sign(temp.determinant());
            Matrix3f R = svd.matrixV()*d*svd.matrixU().transpose();

            // Estimate translation
            Vector3f T = centroidFixed - R*centroidMoving;

            updateTransform.linear() = R;
            updateTransform.translation() = T;
        } else {
            // Only translation
            Vector3f T = centroidFixed - centroidMoving;
            updateTransform.translation() = T;
        }

        // Update current transformation
        currentTransformation = updateTransform*currentTransformation;

        // Calculate RMS error
        MatrixXf distance = rearrangedFixedPoints - currentTransformation*(movingPoints.colwise().homogeneous());
        error = 0;
        for(uint i = 0; i < distance.cols(); i++) {
            error += pow(distance.col(i).norm(),2);
        }
        error = sqrt(error / distance.cols());

        iterations++;
        std::cout << "Error: " << error << std::endl;
        // To continue, change in error has to be above min error change and nr of iterations less than max iterations
    } while(previousError-error > mMinErrorChange && iterations < mMaxIterations);


    if(invertTransform){
        currentTransformation = currentTransformation.inverse();
    }

    mError = error;
    mTransformation = currentTransformation;
}


}
