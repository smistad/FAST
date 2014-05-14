#include "catch.hpp"
#include "MetaImageStreamer.hpp"

using namespace fast;

TEST_CASE("No filename format given to MetaImageStreamer", "[fast][MetaImageStreamer]") {
    MetaImageStreamer::pointer mhdStreamer = MetaImageStreamer::New();
    DynamicImage::pointer image = mhdStreamer->getOutput();
    CHECK_THROWS(image->update());
}

TEST_CASE("No hash tag in filename format given to MetaImageStreamer", "[fast][MetaImageStreamer]") {
    MetaImageStreamer::pointer mhdStreamer = MetaImageStreamer::New();
    CHECK_THROWS(mhdStreamer->setFilenameFormat("asd"));
}

/*
// TODO fix this, will not work because it is another thread that throws the exception
TEST_CASE("Wrong filename format given to MetaImageStreamer", "[fast][MetaImageStreamer]") {
    MetaImageStreamer::pointer mhdStreamer = MetaImageStreamer::New();
    mhdStreamer->setFilenameFormat("asd#asd");
    DynamicImage::pointer image = mhdStreamer->getOutput();
    std::cout << "asd" << std::endl;
    CHECK_THROWS(image->update());
}
*/

TEST_CASE("MetaImageStreamer", "[fast][MetaImageStreamer]") {
    CHECK_NOTHROW(
        MetaImageStreamer::pointer mhdStreamer = MetaImageStreamer::New();
        mhdStreamer->setFilenameFormat(std::string(FAST_ROOT_DIR)+"TestData/US-3Dt/US-3Dt_#.mhd");
        DynamicImage::pointer image = mhdStreamer->getOutput();
        while(!mhdStreamer->hasReachedEnd()) {
            image->update();
        }
    );
}
