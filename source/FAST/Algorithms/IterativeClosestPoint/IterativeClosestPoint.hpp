#pragma once

#include "FAST/ProcessObject.hpp"
#include "FAST/Data/Mesh.hpp"

namespace fast {

/**
 * @brief Registration of two meshes using ICP algorithm
 *
 * @ingroup registration
 */
class FAST_EXPORT  IterativeClosestPoint : public ProcessObject {
    FAST_PROCESS_OBJECT(IterativeClosestPoint)
    public:
        typedef enum { RIGID, TRANSLATION } TransformationType;
        FAST_CONSTRUCTOR(IterativeClosestPoint,
                         TransformationType, type, = RIGID,
                         int, maxIterations, = 100,
                         float, minErrorChange, = 1e-5,
                         float, distanceThreshold, = -1,
                         int, randomSamplingPoints, = 0
        )
        FAST_CONNECT(IterativeClosestPoint, Fixed, 0);
        FAST_CONNECT(IterativeClosestPoint, Moving, 1);
        void setFixedMeshPort(DataChannel::pointer port);
        void setFixedMesh(Mesh::pointer data);
        void setMovingMeshPort(DataChannel::pointer port);
        void setMovingMesh(Mesh::pointer data);
        void setTransformationType(const IterativeClosestPoint::TransformationType type);
        Transform::pointer getOutputTransformation();
        float getError() const;
        void setMinimumErrorChange(float errorChange);
        void setMaximumNrOfIterations(uint iterations);
        void setRandomPointSampling(uint nrOfPointsToSample);
        void setDistanceThreshold(float distance);
    private:
        void execute();

        float mMinErrorChange;
        uint mMaxIterations;
        int mRandomSamplingPoints;
        float mDistanceThreshold;
        float mError;
        Transform::pointer mTransformation;
        IterativeClosestPoint::TransformationType mTransformationType;
};

} // end namespace fast
