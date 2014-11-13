#ifndef ITERATIVE_CLOSEST_POINT_HPP
#define ITERATIVE_CLOSEST_POINT_HPP

#include "ProcessObject.hpp"
#include "LinearTransformation.hpp"
#include "PointSet.hpp"

namespace fast {

class IterativeClosestPoint : public ProcessObject {
    FAST_OBJECT(IterativeClosestPoint)
    public:
        void setFixedPointSet(const PointSet::pointer fixedSet);
        void setMovingPointSet(const PointSet::pointer movingSet);
        LinearTransformation getOutputTransformation();
        float getError() const;
    private:
        IterativeClosestPoint();
        void execute();

        float mMinErrorChange;
        uint mMaxIterations;
        float mError;
};

} // end namespace fast

#endif
