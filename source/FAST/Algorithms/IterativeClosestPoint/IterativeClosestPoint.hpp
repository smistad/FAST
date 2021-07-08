#pragma once

#include "FAST/ProcessObject.hpp"
#include "FAST/Data/Mesh.hpp"

namespace fast {

class FAST_EXPORT  IterativeClosestPoint : public ProcessObject {
    FAST_OBJECT(IterativeClosestPoint)
    public:
        typedef enum { RIGID, TRANSLATION } TransformationType;
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
        IterativeClosestPoint();
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
