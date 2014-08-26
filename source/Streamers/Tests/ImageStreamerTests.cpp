#include "catch.hpp"
#include "ImageStreamer.hpp"
#include "DeviceManager.hpp"

using namespace fast;

TEST_CASE("No filename format given to ImageStreamer", "[fast][ImageStreamer]") {
    ImageStreamer::pointer mhdStreamer = ImageStreamer::New();
    DynamicImage::pointer image = mhdStreamer->getOutput();
    CHECK_THROWS(image->update());
}

TEST_CASE("No hash tag in filename format given to ImageStreamer", "[fast][ImageStreamer]") {
    ImageStreamer::pointer mhdStreamer = ImageStreamer::New();
    CHECK_THROWS(mhdStreamer->setFilenameFormat("asd"));
}

TEST_CASE("Default streaming mode is NEWEST_FRAME_ONLY in ImageStreamer", "[fast][ImageStreamer]") {
    ImageStreamer::pointer mhdStreamer = ImageStreamer::New();
    CHECK(mhdStreamer->getStreamingMode() == STREAMING_MODE_NEWEST_FRAME_ONLY);
}
