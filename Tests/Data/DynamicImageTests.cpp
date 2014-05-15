#include "catch.hpp"
#include "DynamicImage.hpp"

using namespace fast;

namespace fast {

class DummyStreamer : public Streamer {
    FAST_OBJECT(DummyStreamer)
    public:
        void producerStream() {};
        bool hasReachedEnd() const {return mHasReachedEnd;};
    private:
        void execute() {};
        bool mHasReachedEnd;

};

} // end namespace fast

TEST_CASE("New dynamic image has size 0 and has not reached end", "[fast][DynamicImage]") {
    DynamicImage::pointer image = DynamicImage::New();
    CHECK(image->getSize() == 0);
    CHECK(image->hasReachedEnd() == false);
}

TEST_CASE("Dynamic image must have streamer set before it can be used", "[fast][DynamicImage]") {
    DynamicImage::pointer image = DynamicImage::New();
    Image::pointer frame = Image::New();
    CHECK_THROWS(image->addFrame(frame));
}

TEST_CASE("Dynamic image can get and set streamer", "[fast][DynamicImage]") {
    DynamicImage::pointer image = DynamicImage::New();
    DummyStreamer::pointer streamer = DummyStreamer::New();
    image->setStreamer(streamer);
    CHECK(image->getStreamer() == streamer);
}

