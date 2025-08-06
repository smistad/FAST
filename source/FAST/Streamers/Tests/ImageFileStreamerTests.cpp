#include "FAST/Testing.hpp"
#include "FAST/Streamers/ImageFileStreamer.hpp"

using namespace fast;

TEST_CASE("No hash tag in filename format given to ImageFileStreamer", "[fast][ImageFileStreamer]") {
    CHECK_THROWS(ImageFileStreamer::create("asd"));
}

