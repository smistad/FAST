#include "FAST/Algorithms/CoherentPointDrift/CoherentPointDrift.hpp"
#include "FAST/SceneGraph.hpp"
#include "CoherentPointDrift.hpp"

#undef min
#undef max
#include <limits>
#include <random>
#include <unordered_set>

//#include <Eigen/SVD>
#include <iostream>

namespace fast {

    CoherentPointDrift::CoherentPointDrift() {
        createInputPort<Mesh>(0);
        createInputPort<Mesh>(1);
        createOutputPort<Mesh>(0);
        variance = 0.25;             // How to determine this value?
        w = 0.0;                    // How to determine this value?
        mIteration = 0;
        mMaxIterations = 100;
        mTolerance = 0.001;
        mMinErrorChange = 1e-5;
        mError = -1;
        mRandomSamplingPoints = 0;
        mDistanceThreshold = -1;
        mTransformationType = CoherentPointDrift::RIGID;
        mIsModified = true;
        mTransformation = AffineTransformation::New();
    }


    void CoherentPointDrift::setFixedMeshPort(DataPort::pointer port) {
        setInputConnection(0, port);
    }

    void CoherentPointDrift::setMovingMeshPort(DataPort::pointer port) {
        setInputConnection(1, port);
    }

    void CoherentPointDrift::setFixedMesh(Mesh::pointer data) {
        setInputData(0, data);
    }

    void CoherentPointDrift::setMovingMesh(Mesh::pointer data) {
        setInputData(1, data);
    }

    AffineTransformation::pointer CoherentPointDrift::getOutputTransformation() {
        return mTransformation;
    }

    void CoherentPointDrift::execute() {
        auto fixedMesh = getInputData<Mesh>(0);
        auto movingMesh = getInputData<Mesh>(1);

        // Get access to the two point sets
        MeshAccess::pointer accessFixedSet = fixedMesh->getMeshAccess(ACCESS_READ);
        MeshAccess::pointer accessMovingSet = movingMesh->getMeshAccess(ACCESS_READ);

        // Get the points from the meshes
        std::vector<MeshVertex> fixedVertices = accessFixedSet->getVertices();
        std::vector<MeshVertex> movingVertices = accessMovingSet->getVertices();

//        unsigned int numFixedPoints = fixedVertices.size();
//        unsigned int numMovingPoints = movingVertices.size();
        numFixedPoints = (unsigned int)fixedVertices.size();
        numMovingPoints = (unsigned int)movingVertices.size();
        numDimensions = (unsigned int)fixedVertices[0].getPosition().size();

        MatrixXf fixedPoints;
        MatrixXf movingPoints;
        fixedPoints = MatrixXf::Zero(3, fixedVertices.size());
        movingPoints = MatrixXf::Zero(3, movingVertices.size());
        for(int i = 0; i < numFixedPoints; ++i) {
            fixedPoints.col(i) = fixedVertices[i].getPosition();
        }
        for(int i = 0; i < numMovingPoints; ++i) {
            movingPoints.col(i) = movingVertices[i].getPosition();
        }

        fixedPoints.transposeInPlace();         // numFixedPoints x numDimensions
        movingPoints.transposeInPlace();        // numMovingPoints x numDimensions

        // Testing
        std::cout << "****************************************\n";
        std::cout << "Testing av output" << std::endl;
        std::cout << "numFixedPoints = " << numFixedPoints
                  << ", numMovingPoints = " << numMovingPoints << std::endl;
        std::cout << "Dimension = " << numDimensions << std::endl;
        std::cout << "fixedPoints: " << fixedPoints.rows() << ", " << fixedPoints.cols() << std::endl;

        // Initialize the probability matrix of correspondences
        probabilityMatrix = MatrixXf::Zero(numMovingPoints, numFixedPoints);


        // Find the transform here
        while (mIteration < 1 /*mIteration < mMaxIterations && mError > mMinErrorChange*/) {
            expectation(&probabilityMatrix, &fixedPoints, &movingPoints);
            maximization(&probabilityMatrix, &fixedPoints, &movingPoints);
            mIteration++;
        }

        auto transform = AffineTransformation::New();
        Affine3f affine = Affine3f::Identity();
        affine.translate(Vector3f(0, 0, 0));
        transform->setTransform(affine);
        movingMesh->getSceneGraphNode()->setTransformation(transform);

        addOutputData(0, movingMesh);
    }

    void CoherentPointDrift::expectation(
                    MatrixXf* probabilityMatrix,
                    MatrixXf* fixedPoints, MatrixXf* movingPoints) {

        MatrixXf dist = MatrixXf::Zero(numMovingPoints, numFixedPoints);
        for(int i = 0; i < numMovingPoints; ++i) {
//            MatrixXf movingPointMatrix = movingPoints->col(i).replicate(1, numFixedPoints);
            MatrixXf movingPointMatrix = movingPoints->row(i).replicate(numFixedPoints, 1);
            dist = *fixedPoints - movingPointMatrix;
            dist = dist.cwiseAbs2();                                // Square distance components (3xN)
//            probabilityMatrix->row(i) = dist.colwise().sum();       // Sum x, y, z components (1xN)
            probabilityMatrix->row(i) = dist.rowwise().sum();       // Sum x, y, z components (1xN)
        }

        // Normal distribution
        double c = pow(2*(double)EIGEN_PI*variance, (double)numDimensions/2)
                   * (w/(1-w)) * ((double)numMovingPoints/(double)numFixedPoints);

        *probabilityMatrix *= -1/(2 * variance);
        *probabilityMatrix = probabilityMatrix->array().exp();

        MatrixXf denominatorRow = probabilityMatrix->colwise().sum();
        denominatorRow =  denominatorRow.array() + c;

        MatrixXf shouldBeLargerThanEpsilon = Eigen::NumTraits<float>::epsilon() * MatrixXf::Ones(1, numFixedPoints);
        denominatorRow = denominatorRow.cwiseMax(shouldBeLargerThanEpsilon);

        MatrixXf denominator = denominatorRow.replicate(numMovingPoints, 1);

        *probabilityMatrix = probabilityMatrix->cwiseQuotient(denominator);
    }

    void CoherentPointDrift::maximization(MatrixXf* probabilityMatrix,
            MatrixXf* fixedPoints, MatrixXf* movingPoints) {

        // Update transform
        pt1 = probabilityMatrix->transpose().rowwise().sum();
        p1 = probabilityMatrix->rowwise().sum();
        np = probabilityMatrix->sum();

        MatrixXf muX = fixedPoints->transpose() * pt1;
        MatrixXf muY = movingPoints->transpose() * p1;
        MatrixXf fixedPointsPred = *fixedPoints - muX.transpose().replicate(numFixedPoints, 1);
        MatrixXf movingPointsPred = *movingPoints - muY.transpose().replicate(numMovingPoints, 1);
        const MatrixXf A = fixedPointsPred.transpose() * probabilityMatrix->transpose() * movingPointsPred;

        // Singular Value Decomposition (SVD)
        auto svdU =  A.bdcSvd(Eigen::ComputeFullU);
        std::cout << "S before compute V:\n" << svdU.singularValues() << std::endl;
        auto svdV =  A.bdcSvd(Eigen::ComputeFullV);
        const MatrixXf U = svdU.matrixU();
        const MatrixXf V = svdV.matrixV();
        auto S = svdU.singularValues();
        std::cout << "A:\n" << A << std::endl;
        std::cout << "U:\n" << U << std::endl;
        std::cout << "S from svdU after computing U and V:\n" << svdU.singularValues() << std::endl;
        std::cout << "S from svdV after computing U and V:\n" << svdV.singularValues() << std::endl;
        std::cout << "V:\n" << V << std::endl;

        MatrixXf ATest = U * S.asDiagonal() * V.transpose();
        std::cout << "ATest = USV^T:\n" << A << std::endl;

//        auto C = MatrixXf::Ones(1, )
//        MatrixXf R = U *

        std::cout << "np = " << np << std::endl;
    }

    void CoherentPointDrift::setTransformationType(const CoherentPointDrift::TransformationType type) {
        mTransformationType = type;
    }
}