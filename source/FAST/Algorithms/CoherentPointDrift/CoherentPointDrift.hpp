#ifndef COHERENT_POINT_DRIFT_HPP
#define COHERENT_POINT_DRIFT_HPP

#include "FAST/AffineTransformation.hpp"
#include "FAST/ProcessObject.hpp"
#include "FAST/Data/Mesh.hpp"

namespace fast {

    class FAST_EXPORT  CoherentPointDrift: public ProcessObject {
    FAST_OBJECT(CoherentPointDrift)
    public:
        typedef enum { RIGID, TRANSLATION } TransformationType;
        void setFixedMeshPort(DataPort::pointer port);
        void setFixedMesh(Mesh::pointer data);
        void setMovingMeshPort(DataPort::pointer port);
        void setMovingMesh(Mesh::pointer data);
        void setTransformationType(const CoherentPointDrift::TransformationType type);
        AffineTransformation::pointer getOutputTransformation();
    private:
        CoherentPointDrift();
        void expectation(MatrixXf* probabilityMatrix,
                         MatrixXf* fixedPoints, MatrixXf* movingPoints);
        void maximization(MatrixXf* probabilityMatrix,
                          MatrixXf* fixedPoints, MatrixXf* movingPoints);
        void execute();

        MatrixXf probabilityMatrix;
        MatrixXf pt1;
        MatrixXf p1;
        float np;
        unsigned int numFixedPoints;            // N
        unsigned int numMovingPoints;           // M
        unsigned int numDimensions;             // D
        float variance;                         // sigma^2
        float w;
        float mMinErrorChange;
        float mTolerance;
        unsigned char mIteration;
        unsigned char mMaxIterations;
        int mRandomSamplingPoints;
        float mDistanceThreshold;
        float mError;
        AffineTransformation::pointer mTransformation;
        CoherentPointDrift::TransformationType mTransformationType;
    };

} // end namespace fast

#endif
