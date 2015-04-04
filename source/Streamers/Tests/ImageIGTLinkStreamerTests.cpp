#include "catch.hpp"
#include "ImageIGTLinkStreamer.hpp"

using namespace fast;

TEST_CASE("", "[fast][IGTLink]") {

    ImageIGTLinkStreamer::pointer streamer = ImageIGTLinkStreamer::New();
    streamer->setConnectionAddress("localhost");
    streamer->setConnectionPort(18944);
    streamer->update();

}
