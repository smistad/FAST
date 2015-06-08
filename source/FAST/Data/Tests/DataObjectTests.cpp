#include "FAST/Tests/catch.hpp"
#include "FAST/Tests/DummyObjects.hpp"
#include "FAST/DeviceManager.hpp"

namespace fast {

TEST_CASE("Update timestamp on DataObject", "[fast][DataObject]") {
    DummyDataObject::pointer data = DummyDataObject::New();
    unsigned long timestamp = data->getTimestamp();
    data->updateModifiedTimestamp();

    CHECK(timestamp != data->getTimestamp());
}

TEST_CASE("Calling release on a DataObject that has not been retained by any devices throws an exception", "[fast][DataObject]") {
    DeviceManager& deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager.getOneOpenCLDevice();
    DummyDataObject::pointer data = DummyDataObject::New();
    CHECK_THROWS(data->release(device));
}

TEST_CASE("Calling retain and then release on DataObject triggers free on that device", "[fast][DataObject]") {
    DeviceManager& deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager.getOneOpenCLDevice();
    DummyDataObject::pointer data = DummyDataObject::New();
    data->retain(device);
    data->release(device);
    CHECK(data->hasBeenFreed(device) == true);
}

TEST_CASE("Calling retain on DataObject on two devices and then release on one device only triggers free on that device", "[fast][DataObject]") {
    DeviceManager& deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager.getOneOpenCLDevice();
    DummyDataObject::pointer data = DummyDataObject::New();
    Host::pointer host = Host::getInstance();
    data->retain(device);
    data->retain(host);
    data->release(host);
    CHECK(data->hasBeenFreed(device) == false);
    CHECK(data->hasBeenFreed(host) == true);
}

TEST_CASE("Calling retain twice on DataObject and then release once should not trigger free on that device", "[fast][DataObject]") {
    DeviceManager& deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager.getOneOpenCLDevice();
    DummyDataObject::pointer data = DummyDataObject::New();
    data->retain(device);
    data->retain(device);
    data->release(device);
    CHECK(data->hasBeenFreed(device) == false);
}

TEST_CASE("Calling retain twice on DataObject and then release twice triggers free on that device", "[fast][DataObject]") {
    DeviceManager& deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager.getOneOpenCLDevice();
    DummyDataObject::pointer data = DummyDataObject::New();
    data->retain(device);
    data->retain(device);
    data->release(device);
    data->release(device);
    CHECK(data->hasBeenFreed(device) == true);
}


};
