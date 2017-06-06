#include "FAST/Algorithms/IterativeClosestPoint/IterativeClosestPoint.hpp"
#include "FAST/SceneGraph.hpp"
#undef min
#undef max
#include <limits>
#include <random>
#include <unordered_set>

namespace fast {

IterativeClosestPoint::IterativeClosestPoint() {
    createInputPort<Mesh>(0);
    createInputPort<Mesh>(1);
    mMaxIterations = 100;
    mMinErrorChange = 1e-5;
    mError = -1;
    mRandomSamplingPoints = 0;
    mDistanceThreshold = -1;
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

inline double colorDistance(Vector3f e1, Vector3f e2) {
    e1 *= 255;
    e2 *= 255;
    long rmean = ( (long)e1.x() + (long)e2.x() ) / 2;
    long r = (long)e1.x() - (long)e2.x();
    long g = (long)e1.y() - (long)e2.y();
    long b = (long)e1.z() - (long)e2.z();
    return sqrt((((512+rmean)*r*r)>>8) + 4*g*g + (((767-rmean)*b*b)>>8));
}

/**
 * Convert RGB vector to YIQ color space.
 * Y is luminance, while I and Q represent the color in a 2D space.
 * @param rgb
 * @return
 */
inline Vector3f RGB2YIQ(Vector3f rgb) {
    Matrix3f matrix;
    matrix << 0.299, 0.587, 0.114,
        0.596, -0.274, -0.322,
        0.211, -0.523, 0.312;
    return matrix*rgb;
}

/**
 * Create a new matrix which is matrix A rearranged.
 * This matrix has the same size as B
 */
inline MatrixXf rearrangeMatrixToClosestPoints(const MatrixXf& A, const MatrixXf& B, const MatrixXf& Acolors, const MatrixXf& Bcolors, float colorWeight) {
    MatrixXf result = MatrixXf::Constant(B.rows(), B.cols(), 0);

    Vector3f colorWeights(100.0, 1000.0, 1000.0);
    // For each point in B, find the closest point in A
#pragma omp parallel for
    for(int b = 0; b < B.cols(); ++b) {
        Vector3f pointInB = B.col(b);
        Vector3f Bcolor = RGB2YIQ(Bcolors.col(b));
        float minDistance = std::numeric_limits<float>::max();
        uint closestPoint = 0;
        for(int a = 0; a < A.cols(); ++a) {
            Vector3f pointInA = A.col(a);
            Vector3f Acolor = RGB2YIQ(Acolors.col(a));
            VectorXf distanceVector = VectorXf::Zero(6);
            distanceVector.head(3) = pointInA - pointInB;
            distanceVector(3) = (Acolor.x() - Bcolor.x())*colorWeights.x();
            distanceVector(4) = (Acolor.y() - Bcolor.y())*colorWeights.y();
            distanceVector(5) = (Acolor.z() - Bcolor.z())*colorWeights.z();
            //std::cout << distanceVector.tail(3) << std::endl;
            float distance = distanceVector.norm();
            //float distance = (pointInA-pointInB).norm();
            //std::cout << colorDistance(Acolor, Bcolor) << std::endl;
            //if(colorDistance(Acolor, Bcolor) < 100) {
                if(distance < minDistance) {
                    minDistance = distance;
                    closestPoint = a;
                }
            //}
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
    Mesh::pointer fixedMesh = getStaticInputData<Mesh>(0);
    Mesh::pointer movingMesh = getStaticInputData<Mesh>(1);

    // Get access to the two point sets
    MeshAccess::pointer accessFixedSet = fixedMesh->getMeshAccess(ACCESS_READ);
    MeshAccess::pointer accessMovingSet = movingMesh->getMeshAccess(ACCESS_READ);

    // Get transformations of point sets
    AffineTransformation::pointer fixedPointTransform2 = SceneGraph::getAffineTransformationFromData(fixedMesh);
    Affine3f fixedPointTransform;
    fixedPointTransform.matrix() = fixedPointTransform2->getTransform().matrix();
    AffineTransformation::pointer initialMovingTransform2 = SceneGraph::getAffineTransformationFromData(movingMesh);
    Affine3f initialMovingTransform;
    initialMovingTransform.matrix() = initialMovingTransform2->getTransform().matrix();

    // These matrices are 3xN, where N is number of vertices
    std::vector<MeshVertex> fixedVertices = accessFixedSet->getVertices();
    std::vector<MeshVertex> movingVertices = accessMovingSet->getVertices();
    MatrixXf movingPoints;
    MatrixXf fixedPoints;
    MatrixXf movingColors;
    MatrixXf fixedColors;

    // Select from moving
    if(mRandomSamplingPoints > 0) {
        std::vector<MeshVertex> filteredMovingPoints;
        if(movingVertices.size() > mRandomSamplingPoints) {
            std::default_random_engine distributionEngine;
            std::uniform_int_distribution<int> distribution(0, movingVertices.size() - 1);
            int samplesLeft = mRandomSamplingPoints;
            std::unordered_set<int> usedIndices;
            while(samplesLeft > 0) {
                int index = distribution(distributionEngine);
                if(usedIndices.count(index) > 0)
                    continue;
                filteredMovingPoints.push_back(movingVertices[index]);
                usedIndices.insert(index);
                --samplesLeft;
            }
        } else {
            filteredMovingPoints = movingVertices;
        }
        movingPoints = MatrixXf::Zero(3, filteredMovingPoints.size());
        movingColors = MatrixXf::Zero(3, filteredMovingPoints.size());
        for(int i = 0; i < filteredMovingPoints.size(); ++i) {
            movingPoints.col(i) = filteredMovingPoints[i].getPosition();
            movingColors.col(i) = filteredMovingPoints[i].getColor().asVector();
        }
    } else {
        // Select all moving points
        movingPoints = MatrixXf::Zero(3, movingVertices.size());
        movingColors = MatrixXf::Zero(3, movingVertices.size());
        for(int i = 0; i < movingVertices.size(); ++i) {
            movingPoints.col(i) = movingVertices[i].getPosition();
            movingColors.col(i) = movingVertices[i].getColor().asVector();
        }
    }
    movingPoints = initialMovingTransform*movingPoints.colwise().homogeneous();

    // Select from fixed
    if(mDistanceThreshold > 0) {
        Vector3f centroid = getCentroid(movingPoints);
        std::vector<MeshVertex> filteredFixedPoints;
        for(int i = 0; i < fixedVertices.size(); ++i) {
            if ((centroid - fixedVertices[i].getPosition()).norm() < mDistanceThreshold)
                filteredFixedPoints.push_back(fixedVertices[i]);
        }

        if(mRandomSamplingPoints > 0 && mRandomSamplingPoints < filteredFixedPoints.size()) {
            std::default_random_engine distributionEngine;
            std::uniform_int_distribution<int> distribution(0, filteredFixedPoints.size() - 1);
            int samplesLeft = mRandomSamplingPoints;
            std::vector<MeshVertex> newFixedPoints;
            std::unordered_set<int> usedIndices;
            while(samplesLeft > 0) {
                int index = distribution(distributionEngine);
                if(usedIndices.count(index) > 0)
                    continue;
                newFixedPoints.push_back(filteredFixedPoints[index]);
                usedIndices.insert(index);
                --samplesLeft;
            }

            filteredFixedPoints = newFixedPoints;
        }
        fixedPoints = MatrixXf::Zero(3, filteredFixedPoints.size());
        fixedColors = MatrixXf::Zero(3, filteredFixedPoints.size());
        for(int i = 0; i < filteredFixedPoints.size(); ++i) {
            fixedPoints.col(i) = filteredFixedPoints[i].getPosition();
            fixedColors.col(i) = filteredFixedPoints[i].getColor().asVector();
        }

        reportInfo() << fixedVertices.size() << " points reduced to " << filteredFixedPoints.size() << reportEnd();
    } else {
        fixedPoints = MatrixXf::Zero(3, fixedVertices.size());
        fixedColors = MatrixXf::Zero(3, fixedVertices.size());
        for(int i = 0; i < fixedVertices.size(); ++i) {
            fixedPoints.col(i) = fixedVertices[i].getPosition();
            fixedColors.col(i) = fixedVertices[i].getColor().asVector();
        }
    }
    Affine3f currentTransformation = Affine3f::Identity();
    if(fixedPoints.size() == 0 || movingPoints.size() == 0) {
        mTransformation->setTransform(currentTransformation);
        return;
    }
    fixedPoints = fixedPointTransform*fixedPoints.colwise().homogeneous();

    float colorWeight = 1.0f;

    // Want to choose the smallest one as moving
    bool invertTransform = false;
	MatrixXf movedPoints = currentTransformation*(movingPoints.colwise().homogeneous());
    // Match closest points using current transformation
    MatrixXf rearrangedFixedPoints = rearrangeMatrixToClosestPoints(
            fixedPoints, movedPoints, fixedColors, movingColors, colorWeight);
    do {
        previousError = error;        

        reportInfo() << "Processing " << rearrangedFixedPoints.cols() << " points in ICP" << reportEnd();
        // Get centroids
        Vector3f centroidFixed = getCentroid(rearrangedFixedPoints);
        //reportInfo() << "Centroid fixed: " << Reporter::end();
        //reportInfo() << centroidFixed << Reporter::end();
        Vector3f centroidMoving = getCentroid(movedPoints);
        //reportInfo() << "Centroid moving: " << Reporter::end();
        //reportInfo() << centroidMoving << Reporter::end();

        Eigen::Affine3f updateTransform = Eigen::Affine3f::Identity();

        if(mTransformationType == IterativeClosestPoint::RIGID) {
            // See http://se.mathworks.com/matlabcentral/fileexchange/27804-iterative-closest-point for ref
            // eq_point
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
        // Should we rearrange the points here?
        mRuntimeManager->startRegularTimer("find_closest");
        rearrangedFixedPoints = rearrangeMatrixToClosestPoints(
                fixedPoints, movedPoints, fixedColors, movingColors, colorWeight);
        mRuntimeManager->stopRegularTimer("find_closest");
		MatrixXf distance = rearrangedFixedPoints - movedPoints;
        error = 0;
        for(uint i = 0; i < distance.cols(); i++) {
            error += square(distance.col(i).norm());
        }
        error = sqrt(error / distance.cols());

        iterations++;
        reportInfo() << "ICP error: " << error << Reporter::end();
        // To continue, change in error has to be above min error change and nr of iterations less than max iterations
    } while(previousError-error > mMinErrorChange && iterations < mMaxIterations);


    if(invertTransform){
        currentTransformation = currentTransformation.inverse();
    }
    reportInfo() << "Final transform: " << currentTransformation.matrix() << reportEnd();

    mError = error;
    mTransformation->setTransform(currentTransformation);
}

void IterativeClosestPoint::setMaximumNrOfIterations(uint iterations) {
    if(iterations == 0)
        throw Exception("Max nr of iterations can't be 0");
    mMaxIterations = iterations;
}

void IterativeClosestPoint::setRandomPointSampling(uint nrOfPointsToSample) {
    mRandomSamplingPoints = nrOfPointsToSample;
}

void IterativeClosestPoint::setDistanceThreshold(float distance) {
    mDistanceThreshold = distance;
}

void IterativeClosestPoint::setMinimumErrorChange(float errorChange) {
    mMinErrorChange = errorChange;
}

}
