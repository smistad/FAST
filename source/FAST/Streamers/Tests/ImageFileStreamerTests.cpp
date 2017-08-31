#include "FAST/Testing.hpp"
#include "FAST/DeviceManager.hpp"
#include <thread>
#include "FAST/Streamers/ImageFileStreamer.hpp"
#include "FAST/Tests/DummyObjects.hpp"
#include "FAST/Data/Image.hpp"

using namespace fast;

TEST_CASE("No filename format given to ImageFileStreamer", "[fast][ImageFileStreamer]") {
    ImageFileStreamer::pointer mhdStreamer = ImageFileStreamer::New();
    CHECK_THROWS(mhdStreamer->update(0));
}

TEST_CASE("No hash tag in filename format given to ImageFileStreamer", "[fast][ImageFileStreamer]") {
    ImageFileStreamer::pointer mhdStreamer = ImageFileStreamer::New();
    CHECK_THROWS(mhdStreamer->setFilenameFormat("asd"));
}

