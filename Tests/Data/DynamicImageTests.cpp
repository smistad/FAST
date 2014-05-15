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

TEST_CASE("getNextFrame on dynamic image with size 0 returns exception", "[fast][DynamicImage]") {
    DynamicImage::pointer image = DynamicImage::New();
    DummyStreamer::pointer streamer = DummyStreamer::New();
    image->setStreamer(streamer);
    CHECK_THROWS(image->getNextFrame());
}

TEST_CASE("Addming frames to dynamic image updates the timestamp of the dynamic image" "[fast][DynamicImage]") {
    DynamicImage::pointer image = DynamicImage::New();
    DummyStreamer::pointer streamer = DummyStreamer::New();
    streamer->setStreamingMode(STREAMING_MODE_NEWEST_FRAME_ONLY);
    image->setStreamer(streamer);
    unsigned long timestamp = image->getTimestamp();
    Image::pointer frame = Image::New();
    image->addFrame(frame);
    CHECK(image->getTimestamp() != timestamp);
}

TEST_CASE("Adding frames to dynamic image does not change size for streaming mode NEWEST_FRAME_ONLY", "[fast][DynamicImage]") {
    DynamicImage::pointer image = DynamicImage::New();
    DummyStreamer::pointer streamer = DummyStreamer::New();
    streamer->setStreamingMode(STREAMING_MODE_NEWEST_FRAME_ONLY);
    image->setStreamer(streamer);

    Image::pointer frame = Image::New();
    Image::pointer frame2 = Image::New();
    image->addFrame(frame);
    CHECK(image->getSize() == 1);
    image->addFrame(frame2);
    CHECK(image->getSize() == 1);
}

TEST_CASE("Adding frames to dynamic image changes size for streaming mode PROCESS_ALL", "[fast][DynamicImage]") {
    DynamicImage::pointer image = DynamicImage::New();
    DummyStreamer::pointer streamer = DummyStreamer::New();
    streamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
    image->setStreamer(streamer);

    Image::pointer frame = Image::New();
    Image::pointer frame2 = Image::New();
    image->addFrame(frame);
    CHECK(image->getSize() == 1);
    image->addFrame(frame2);
    CHECK(image->getSize() == 2);
}

TEST_CASE("Adding frames to dynamic image changes size for streaming mode STORE_ALL", "[fast][DynamicImage]") {
    DynamicImage::pointer image = DynamicImage::New();
    DummyStreamer::pointer streamer = DummyStreamer::New();
    streamer->setStreamingMode(STREAMING_MODE_STORE_ALL_FRAMES);
    image->setStreamer(streamer);

    Image::pointer frame = Image::New();
    Image::pointer frame2 = Image::New();
    image->addFrame(frame);
    CHECK(image->getSize() == 1);
    image->addFrame(frame2);
    CHECK(image->getSize() == 2);
}

TEST_CASE("Dynamic image with streaming mode NEWEST_FRAME_ONLY always keeps the last frame", "[fast][DynamicImage]") {
    DynamicImage::pointer image = DynamicImage::New();
    DummyStreamer::pointer streamer = DummyStreamer::New();
    streamer->setStreamingMode(STREAMING_MODE_NEWEST_FRAME_ONLY);
    image->setStreamer(streamer);

    CHECK(image->getSize() == 0);
    Image::pointer frame = Image::New();
    image->addFrame(frame);
    Image::pointer frame2 = image->getNextFrame();
    CHECK(image->getSize() == 1);
}

TEST_CASE("Getting next frame changes size with streaming mode PROCESS_ALL", "[fast][DynamicImage]") {
    DynamicImage::pointer image = DynamicImage::New();
    DummyStreamer::pointer streamer = DummyStreamer::New();
    streamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
    image->setStreamer(streamer);

    Image::pointer frame = Image::New();
    Image::pointer frame2 = Image::New();
    image->addFrame(frame);
    image->addFrame(frame2);
    CHECK(image->getSize() == 2);
    image->getNextFrame();
    CHECK(image->getSize() == 1);
    image->getNextFrame();
    CHECK(image->getSize() == 0);
}

TEST_CASE("Getting next frame does not change size with streaming mode STORE_ALL", "[fast][DynamicImage]") {
    DynamicImage::pointer image = DynamicImage::New();
    DummyStreamer::pointer streamer = DummyStreamer::New();
    streamer->setStreamingMode(STREAMING_MODE_STORE_ALL_FRAMES);
    image->setStreamer(streamer);

    Image::pointer frame = Image::New();
    Image::pointer frame2 = Image::New();
    image->addFrame(frame);
    image->addFrame(frame2);
    CHECK(image->getSize() == 2);
    image->getNextFrame();
    CHECK(image->getSize() == 2);
    image->getNextFrame();
    CHECK(image->getSize() == 2);
}

TEST_CASE("Dynamic image with streaming mode NEWEST_FRAME_ONLY always returns the last frame inserted", "[fast][DynamicImage]") {
    DynamicImage::pointer image = DynamicImage::New();
    DummyStreamer::pointer streamer = DummyStreamer::New();
    streamer->setStreamingMode(STREAMING_MODE_NEWEST_FRAME_ONLY);
    image->setStreamer(streamer);

    Image::pointer frame1 = Image::New();
    Image::pointer frame2 = Image::New();
    Image::pointer frame3 = Image::New();
    image->addFrame(frame1);
    CHECK(image->getNextFrame() == frame1);
    image->addFrame(frame2);
    image->addFrame(frame3);
    CHECK(image->getNextFrame() == frame3);
    CHECK(image->getNextFrame() == frame3);
}

TEST_CASE("Dynamic image with streaming mode PROCESS_ALL returns the frames in the order they were added", "[fast][DynamicImage]") {
    DynamicImage::pointer image = DynamicImage::New();
    DummyStreamer::pointer streamer = DummyStreamer::New();
    streamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
    image->setStreamer(streamer);

    Image::pointer frame1 = Image::New();
    Image::pointer frame2 = Image::New();
    Image::pointer frame3 = Image::New();
    image->addFrame(frame1);
    image->addFrame(frame2);
    image->addFrame(frame3);
    CHECK(image->getNextFrame() == frame1);
    CHECK(image->getNextFrame() == frame2);
    CHECK(image->getNextFrame() == frame3);
}

TEST_CASE("Dynamic image with streaming mode STORE_ALL returns the frames in the order they were added", "[fast][DynamicImage]") {
    DynamicImage::pointer image = DynamicImage::New();
    DummyStreamer::pointer streamer = DummyStreamer::New();
    streamer->setStreamingMode(STREAMING_MODE_STORE_ALL_FRAMES);
    image->setStreamer(streamer);

    Image::pointer frame1 = Image::New();
    Image::pointer frame2 = Image::New();
    Image::pointer frame3 = Image::New();
    image->addFrame(frame1);
    image->addFrame(frame2);
    image->addFrame(frame3);
    CHECK(image->getNextFrame() == frame1);
    CHECK(image->getNextFrame() == frame2);
    CHECK(image->getNextFrame() == frame3);
}

TEST_CASE("Dynamic image with streaming mode PROCESS_ALL throws exception when all frames have been retrieved", "[fast][DynamicImage]") {
    DynamicImage::pointer image = DynamicImage::New();
    DummyStreamer::pointer streamer = DummyStreamer::New();
    streamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
    image->setStreamer(streamer);

    Image::pointer frame1 = Image::New();
    Image::pointer frame2 = Image::New();
    Image::pointer frame3 = Image::New();
    image->addFrame(frame1);
    image->addFrame(frame2);
    image->addFrame(frame3);
    image->getNextFrame();
    image->getNextFrame();
    image->getNextFrame();
    CHECK_THROWS(image->getNextFrame());
}

TEST_CASE("Dynamic image with streaming mode STORE_ALL throws exception when all frames have been retrieved", "[fast][DynamicImage]") {
    DynamicImage::pointer image = DynamicImage::New();
    DummyStreamer::pointer streamer = DummyStreamer::New();
    streamer->setStreamingMode(STREAMING_MODE_STORE_ALL_FRAMES);
    image->setStreamer(streamer);

    Image::pointer frame1 = Image::New();
    Image::pointer frame2 = Image::New();
    Image::pointer frame3 = Image::New();
    image->addFrame(frame1);
    image->addFrame(frame2);
    image->addFrame(frame3);
    image->getNextFrame();
    image->getNextFrame();
    image->getNextFrame();
    CHECK_THROWS(image->getNextFrame());
}

// TODO test that hasReachedEnd works and test that timestamp is updated properly with getNextFrame

