#ifndef POINT_SET_ACCESS_HPP_
#define POINT_SET_ACCESS_HPP_

#include "FAST/Data/DataTypes.hpp"
#include "Access.hpp"
#include "FAST/SmartPointers.hpp"

namespace fast {

class PointSetAccess {
    public:
        PointSetAccess(std::vector<Vector3f>* points, bool* accessFlag, bool* accessFlag2);
        Vector3f getPoint(uint i) const;
        void setPoint(uint i, const Vector3f point);
        void addPoint(const Vector3f point);
        MatrixXf getPointSetAsMatrix() const;
        void release();
        ~PointSetAccess();
		typedef UniquePointer<PointSetAccess> pointer;
    private:
        bool* mAccessFlag;
        bool* mAccessFlag2;
        std::vector<Vector3f>* mPointSet;
};

} // end namespace fast

#endif
