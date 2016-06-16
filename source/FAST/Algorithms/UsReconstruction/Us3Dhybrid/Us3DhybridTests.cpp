#include "FAST/Testing.hpp"
#include "Us3Dhybrid.hpp"

//Runs tests like RunUs3Dhybrid.cpp and CHECK results

namespace fast {

TEST_CASE("Us3Dhybrid point2plane intersection: Distance, Crossing, LocalCrossing", "[fast][Us3Dhybrid]") {    
    Us3Dhybrid::pointer pnnHybrid = Us3Dhybrid::New();

    Vector3i startPoint = Vector3i(0, 0, 0);
    Vector3f planeBasePoint = Vector3f(2.f, 2.f, 2.f);
    Vector3f planeNormal = Vector3f(0.f, 0.f, 1.f);
    float distance = pnnHybrid->getPointDistanceAlongNormal(startPoint, planeBasePoint, planeNormal);
    CHECK(2.f == distance);
    Vector3f intersection = pnnHybrid->getIntersectionOfPlane(startPoint, distance, normalVector);
    Vector3f trueIntersection = Vector3f(2.f, 2.f, 0.f);
    CHECK(trueIntersection == intersection); //ev component by component
    //AffineTransformation::pointer inverseTransform;
    //inverseTransform->
    //Vector3f intersectionLocal = getLocalIntersectionOfPlane(intersection, inverseTransform);
    //bool isWithinFrame(Vector3f intersectionPointLocal, Vector3ui frameSize)

    /*std::vector<OpenCLDevice::pointer> devices = DeviceManager::getInstance().getAllDevices();
    for (int i = 0; i < devices.size(); i++) {
        INFO("Device " << devices[i]->getName());
        CHECK(17893 == sum);
    }*/
}

} // end namespace fast