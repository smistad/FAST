#ifndef COHERENT_POINT_DRIFT_HPP
#define COHERENT_POINT_DRIFT_HPP

#include "FAST/AffineTransformation.hpp"
#include "FAST/ProcessObject.hpp"
#include "FAST/Data/Mesh.hpp"

#include "FAST/SmartPointers.hpp"

namespace fast {

    class FAST_EXPORT  CoherentPointDrift: public ProcessObject {
//    FAST_OBJECT(CoherentPointDrift)
    public:
        typedef enum { RIGID, AFFINE, NONRIGID } TransformationType;
        void setFixedMeshPort(DataPort::pointer port);
        void setFixedMesh(Mesh::pointer data);
        void setMovingMeshPort(DataPort::pointer port);
        void setMovingMesh(Mesh::pointer data);
        void setMaximumIterations(unsigned char maxIterations);
        void setUniformWeight(float uniformWeight);
        void setTolerance(double tolerance);
        AffineTransformation::pointer getOutputTransformation();

        virtual void initializeVarianceAndMore() = 0;
        void expectation(MatrixXf& fixedPoints, MatrixXf& movingPoints);
        virtual void maximization(MatrixXf& fixedPoints, MatrixXf& movingPoints) = 0;

    protected:
        CoherentPointDrift();
        void execute();
        MatrixXf mFixedPoints;
        MatrixXf mMovingPoints;
        MatrixXf mMovingMeanInitial;
        MatrixXf mFixedMeanInitial;
        MatrixXf mResponsibilityMatrix;         // P
        unsigned int mNumFixedPoints;           // N
        unsigned int mNumMovingPoints;          // M
        unsigned int mNumDimensions;            // D
        float mUniformWeight;                   // Weight of the uniform distribution
        double mTolerance;                      // Convergence criteria for EM iterations
        double mScale;                          // s
        double mVariance;                       // sigma^2
        double mObjectiveFunction;              // Q
        double mFixedNormalizationScale;
        double mMovingNormalizationScale;
        AffineTransformation::pointer mTransformation;
        unsigned char mIteration;
        bool mRegistrationConverged;
        double timeE;
        double timeEDistances;
        double timeENormal;
        double timeEPosterior;
        double timeEPosteriorDivision;
        double timeM;
        double timeMUseful;
        double timeMCenter;
        double timeMSVD;
        double timeMParameters;
        double timeMUpdate;

    private:
        void initializePointSets();
        void printCloudDimensions();
        void normalizePointSets();

        std::shared_ptr<Mesh> mFixedMesh;
        std::shared_ptr<Mesh> mMovingMesh;
        unsigned char mMaxIterations;
        CoherentPointDrift::TransformationType mTransformationType;
    };

} // end namespace fast

#endif
