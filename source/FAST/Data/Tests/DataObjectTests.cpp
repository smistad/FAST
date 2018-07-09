#include "FAST/Testing.hpp"
#include "FAST/Tests/DummyObjects.hpp"
#include "FAST/DeviceManager.hpp"

namespace fast {

TEST_CASE("Update timestamp on DataObject", "[fast][DataObject]") {
    DummyDataObject::pointer data = DummyDataObject::New();
    unsigned long timestamp = data->getTimestamp();
    data->updateModifiedTimestamp();

    CHECK(timestamp != data->getTimestamp());
}



};
