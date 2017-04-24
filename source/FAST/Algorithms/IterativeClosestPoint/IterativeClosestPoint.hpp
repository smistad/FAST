#ifndef ITERATIVE_CLOSEST_POINT_HPP
#define ITERATIVE_CLOSEST_POINT_HPP

#include "FAST/AffineTransformation.hpp"
#include "FAST/ProcessObject.hpp"
#include "FAST/Data/Mesh.hpp"

namespace fast {

class IterativeClosestPoint : public ProcessObject {
    FAST_OBJECT(IterativeClosestPoint)
    public:
        typedef enum { RIGID, TRANSLATION } TransformationType;
        void setFixedMeshPort(ProcessObjectPort port);
        void setFixedMesh(Mesh::pointer data);
        void setMovingMeshPort(ProcessObjectPort port);
        void setMovingMesh(Mesh::pointer data);
        void setTransformationType(const IterativeClosestPoint::TransformationType type);
        AffineTransformation::pointer getOutputTransformation();
        float getError() const;
        void setMaximumNrOfIterations(uint iterations);
    private:
        IterativeClosestPoint();
        void execute();

        float mMinErrorChange;
        uint mMaxIterations;
        float mError;
        AffineTransformation::pointer mTransformation;
        IterativeClosestPoint::TransformationType mTransformationType;
};

} // end namespace fast

#endif
