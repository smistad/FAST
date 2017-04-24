#include "FAST/Algorithms/IterativeClosestPoint/IterativeClosestPoint.hpp"
#include "FAST/SceneGraph.hpp"
#undef min
#undef max
#include <limits>

namespace fast {

IterativeClosestPoint::IterativeClosestPoint() {
    createInputPort<Mesh>(0);
    createInputPort<Mesh>(1);
    mMaxIterations = 100;
    mMinErrorChange = 1e-5;
    mError = -1;
    mTransformationType = IterativeClosestPoint::RIGID;
    mIsModified = true;
    mTransformation = AffineTransformation::New();
}


void IterativeClosestPoint::setFixedMeshPort(ProcessObjectPort port) {
    setInputConnection(0, port);
}
void IterativeClosestPoint::setMovingMeshPort(ProcessObjectPort port) {
    setInputConnection(1, port);
}
void IterativeClosestPoint::setFixedMesh(Mesh::pointer data) {
    setInputData(0, data);
}
void IterativeClosestPoint::setMovingMesh(Mesh::pointer data) {
    setInputData(1, data);
}

AffineTransformation::pointer IterativeClosestPoint::getOutputTransformation() {
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
    MeshAccess::pointer accessFixedSet = getStaticInputData<Mesh>(0)->getMeshAccess(ACCESS_READ);
    MeshAccess::pointer accessMovingSet =getStaticInputData<Mesh>(1)->getMeshAccess(ACCESS_READ);

    // Get transformations of point sets
    AffineTransformation::pointer fixedPointTransform2 = SceneGraph::getAffineTransformationFromData(getStaticInputData<Mesh>(0));
    Eigen::Affine3f fixedPointTransform;
    fixedPointTransform.matrix() = fixedPointTransform2->matrix();
    AffineTransformation::pointer initialMovingTransform2 = SceneGraph::getAffineTransformationFromData(getStaticInputData<Mesh>(1));
    Eigen::Affine3f initialMovingTransform;
    initialMovingTransform.matrix() = initialMovingTransform2->matrix();

    // These matrices are 3xN, where N is number of vertices
    std::vector<MeshVertex> fixedVertices = accessFixedSet->getVertices();
    std::vector<MeshVertex> movingVertices = accessMovingSet->getVertices();
    MatrixXf movingPoints = MatrixXf::Zero(3, movingVertices.size());

    // TODO filter out points in fixed which are very far away from moving
    Vector3f centroid = Vector3f::Zero();
    for(int i = 0; i < movingVertices.size(); ++i) {
        movingPoints.col(i) = movingVertices[i].getPosition();
        centroid += movingVertices[i].getPosition();
    }
    centroid /= movingVertices.size();

    std::vector<Vector3f> filteredFixedPoints;
    for(int i = 0; i < fixedVertices.size(); ++i) {
        if((centroid - fixedVertices[i].getPosition()).norm() < 10*10)
            filteredFixedPoints.push_back(fixedVertices[i].getPosition());
    }

    MatrixXf fixedPoints = MatrixXf::Zero(3, filteredFixedPoints.size());
    for(int i = 0; i < filteredFixedPoints.size(); ++i) {
        fixedPoints.col(i) = filteredFixedPoints[i];
    }
    reportInfo() << fixedVertices.size() << " points reduced to " << filteredFixedPoints.size() << reportEnd();
    Eigen::Affine3f currentTransformation = Eigen::Affine3f::Identity();
    if(filteredFixedPoints.size() == 0) {
        mTransformation->matrix() = currentTransformation.matrix();
        return;
    }

    // Want to choose the smallest one as moving
    bool invertTransform = false;
    if(false && fixedPoints.cols() < movingPoints.cols()) {
        reportInfo() << "switching fixed and moving" << Reporter::end;
        // Switch fixed and moving
        MatrixXf temp = fixedPoints;
        fixedPoints = movingPoints;
        movingPoints = temp;
        invertTransform = true;

        // Apply initial transformations
        movingPoints = fixedPointTransform*movingPoints.colwise().homogeneous();
        fixedPoints = initialMovingTransform*fixedPoints.colwise().homogeneous();
    } else {
        // Apply initial transformations
        movingPoints = initialMovingTransform*movingPoints.colwise().homogeneous();
        fixedPoints = fixedPointTransform*fixedPoints.colwise().homogeneous();
    }
	MatrixXf movedPoints = currentTransformation*(movingPoints.colwise().homogeneous());
    do {
        previousError = error;        

        // Match closest points using current transformation
        MatrixXf rearrangedFixedPoints = rearrangeMatrixToClosestPoints(
                fixedPoints, movedPoints);

        reportInfo() << "Processing " << rearrangedFixedPoints.cols() << " points in ICP" << reportEnd();
        // Get centroids
        Vector3f centroidFixed = getCentroid(rearrangedFixedPoints);
        //reportInfo() << "Centroid fixed: " << Reporter::end;
        //reportInfo() << centroidFixed << Reporter::end;
        Vector3f centroidMoving = getCentroid(movedPoints);
        //reportInfo() << "Centroid moving: " << Reporter::end;
        //reportInfo() << centroidMoving << Reporter::end;

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

		// Move points
		movedPoints = currentTransformation*(movingPoints.colwise().homogeneous());

        // Calculate RMS error		
		MatrixXf distance = rearrangedFixedPoints - movedPoints;
        error = 0;
        for(uint i = 0; i < distance.cols(); i++) {
            error += pow(distance.col(i).norm(),2);
        }
        error = sqrt(error / distance.cols());

        iterations++;
        reportInfo() << "ICP error: " << error << Reporter::end;
        // To continue, change in error has to be above min error change and nr of iterations less than max iterations
    } while(previousError-error > mMinErrorChange && iterations < mMaxIterations);


    if(invertTransform){
        currentTransformation = currentTransformation.inverse();
    }

    mError = error;
    mTransformation->matrix() = currentTransformation.matrix();
}

void IterativeClosestPoint::setMaximumNrOfIterations(uint iterations) {
    if(iterations == 0)
        throw Exception("Max nr of iterations can't be 0");
    mMaxIterations = iterations;
}

}
