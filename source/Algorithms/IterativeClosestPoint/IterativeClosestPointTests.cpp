#include "catch.hpp"
#include "IterativeClosestPoint.hpp"

namespace fast {

TEST_CASE("IterativeClosestPoint", "[fast][IterativeClosestPoint][icp]") {

    PointSet::pointer A = PointSet::New();
    PointSetAccess accessA = A->getAccess(ACCESS_READ_WRITE);
    accessA.addPoint(Vector3f(2,2,1));
    accessA.addPoint(Vector3f(6,2,2));
    accessA.addPoint(Vector3f(2,6,1));
    accessA.release();

    PointSet::pointer B = PointSet::New();
    PointSetAccess accessB = B->getAccess(ACCESS_READ_WRITE);
    accessB.addPoint(Vector3f(3,2,0));
    accessB.addPoint(Vector3f(7,0,0));
    accessB.addPoint(Vector3f(9,5,0));
    accessB.addPoint(Vector3f(2,1,1));
    accessB.addPoint(Vector3f(2,1,8));
    accessB.release();

    IterativeClosestPoint::pointer icp = IterativeClosestPoint::New();
    icp->setInputData(0, B);
    icp->setInputData(1, A);

    CHECK_NOTHROW(icp->update());

    //icp->getOutputTransformation();

}

} // end namespace fast
