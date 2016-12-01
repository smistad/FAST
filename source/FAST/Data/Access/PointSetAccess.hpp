#ifndef POINT_SET_ACCESS_HPP_
#define POINT_SET_ACCESS_HPP_

#include "FAST/Data/DataTypes.hpp"
#include "Access.hpp"
#include "FAST/SmartPointers.hpp"

namespace fast {

class PointSet;

class PointSetAccess {
    public:
        PointSetAccess(std::vector<Vector3f>* points, SharedPointer<PointSet> object);
        Vector3f getPoint(uint i) const;
        std::vector<Vector3f> getPoints() const;
        void setPoint(uint i, const Vector3f point);
        void addPoint(const Vector3f point);
        MatrixXf getPointSetAsMatrix() const;
        void release();
        ~PointSetAccess();
		typedef UniquePointer<PointSetAccess> pointer;
    private:
        std::vector<Vector3f>* mPointSet;
        SharedPointer<PointSet> mObject;
};

} // end namespace fast

#endif
