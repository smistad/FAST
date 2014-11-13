#include "catch.hpp"
#include "IterativeClosestPoint.hpp"

namespace fast {

TEST_CASE("IterativeClosestPoint", "[fast][IterativeClosestPoint][icp]") {

    PointSet::pointer A = PointSet::New();
    PointSetAccess accessA = A->getAccess(ACCESS_READ_WRITE);
    accessA.addPoint(Vector3f(2,2,0));
    accessA.addPoint(Vector3f(6,2,0));
    accessA.addPoint(Vector3f(6,6,0));
    accessA.release();

    PointSet::pointer B = PointSet::New();
    PointSetAccess accessB = B->getAccess(ACCESS_READ_WRITE);
    accessB.addPoint(Vector3f(3,2,0));
    accessB.addPoint(Vector3f(7,0,0));
    accessB.addPoint(Vector3f(9,5,0));
    accessB.release();

    IterativeClosestPoint::pointer icp = IterativeClosestPoint::New();
    icp->setMovingPointSet(B);
    icp->setFixedPointSet(A);

    icp->update();

    //icp->getOutputTransformation();

}

} // end namespace fast
