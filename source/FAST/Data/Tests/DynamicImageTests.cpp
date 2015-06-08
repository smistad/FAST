#include "FAST/Tests/catch.hpp"
#include "FAST/Data/Image.hpp"
#include "FAST/Streamers/Streamer.hpp"
#include "FAST/ProcessObject.hpp"
#include "FAST/Tests/DummyObjects.hpp"

using namespace fast;

namespace fast {

class DummyStreamer : public Streamer, public ProcessObject {
    FAST_OBJECT(DummyStreamer)
    public:
        void producerStream() {};
        bool hasReachedEnd() const {return mHasReachedEnd;};
        void setReachedEnd() {mHasReachedEnd = true;};
        uint getNrOfFrames() const { return mNrOfFrames; };
        void setNrOfFrames(uint nr) { mNrOfFrames = nr; };
    private:
        DummyStreamer() : mHasReachedEnd(false), mNrOfFrames(0) {};
        void execute() {};
        bool mHasReachedEnd;
        uint mNrOfFrames;

};

} // end namespace fast

TEST_CASE("New dynamic image has size 0 and has not reached end", "[fast][DynamicData]") {
    DynamicData::pointer image = DynamicData::New();
    CHECK(image->getSize() == 0);
}

TEST_CASE("Dynamic image can get and set streamer", "[fast][DynamicData]") {
    DynamicData::pointer image = DynamicData::New();
    DummyStreamer::pointer streamer = DummyStreamer::New();
    image->setStreamer(streamer);
    CHECK(image->getStreamer() == streamer);
}

TEST_CASE("getNextFrame on dynamic image with size 0 returns exception", "[fast][DynamicData]") {
    DynamicData::pointer image = DynamicData::New();
    DummyStreamer::pointer streamer = DummyStreamer::New();
    DummyProcessObject::pointer PO = DummyProcessObject::New();
    image->setStreamer(streamer);
    CHECK_THROWS(image->getNextFrame(PO));
}

TEST_CASE("Adding frames to dynamic image updates the timestamp of the dynamic image" "[fast][DynamicData]") {
    DynamicData::pointer image = DynamicData::New();
    DummyStreamer::pointer streamer = DummyStreamer::New();
    streamer->setStreamingMode(STREAMING_MODE_NEWEST_FRAME_ONLY);
    image->setStreamer(streamer);
    unsigned long timestamp = image->getTimestamp();
    Image::pointer frame = Image::New();
    image->addFrame(frame);
    CHECK(image->getTimestamp() != timestamp);
}

TEST_CASE("Adding frames to dynamic image does not change size for streaming mode NEWEST_FRAME_ONLY", "[fast][DynamicData]") {
    DynamicData::pointer image = DynamicData::New();
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

TEST_CASE("Adding frames to dynamic image changes size for streaming mode PROCESS_ALL", "[fast][DynamicData]") {
    DynamicData::pointer image = DynamicData::New();
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

TEST_CASE("Adding frames to dynamic image changes size for streaming mode STORE_ALL", "[fast][DynamicData]") {
    DynamicData::pointer image = DynamicData::New();
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

TEST_CASE("Dynamic image with streaming mode NEWEST_FRAME_ONLY always keeps the last frame", "[fast][DynamicData]") {
    DynamicData::pointer image = DynamicData::New();
    DummyStreamer::pointer streamer = DummyStreamer::New();
    DummyProcessObject::pointer PO = DummyProcessObject::New();
    streamer->setStreamingMode(STREAMING_MODE_NEWEST_FRAME_ONLY);
    image->setStreamer(streamer);

    CHECK(image->getSize() == 0);
    Image::pointer frame = Image::New();
    image->addFrame(frame);
    Image::pointer frame2 = image->getNextFrame(PO);
    CHECK(image->getSize() == 1);
}

TEST_CASE("Getting next frame changes size with streaming mode PROCESS_ALL", "[fast][DynamicData]") {
    DynamicData::pointer image = DynamicData::New();
    DummyStreamer::pointer streamer = DummyStreamer::New();
    DummyProcessObject::pointer PO = DummyProcessObject::New();
    streamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
    image->setStreamer(streamer);

    Image::pointer frame = Image::New();
    Image::pointer frame2 = Image::New();
    image->addFrame(frame);
    image->addFrame(frame2);
    CHECK(image->getSize() == 2);
    image->getNextFrame(PO);
    CHECK(image->getSize() == 1);
    image->getNextFrame(PO);
    CHECK(image->getSize() == 0);
}

TEST_CASE("Getting next frame does not change size with streaming mode STORE_ALL", "[fast][DynamicData]") {
    DynamicData::pointer image = DynamicData::New();
    DummyStreamer::pointer streamer = DummyStreamer::New();
    DummyProcessObject::pointer PO = DummyProcessObject::New();
    streamer->setStreamingMode(STREAMING_MODE_STORE_ALL_FRAMES);
    image->setStreamer(streamer);

    Image::pointer frame = Image::New();
    Image::pointer frame2 = Image::New();
    image->addFrame(frame);
    image->addFrame(frame2);
    CHECK(image->getSize() == 2);
    image->getNextFrame(PO);
    CHECK(image->getSize() == 2);
    image->getNextFrame(PO);
    CHECK(image->getSize() == 2);
}

TEST_CASE("Dynamic image with streaming mode NEWEST_FRAME_ONLY always returns the last frame inserted", "[fast][DynamicData]") {
    DynamicData::pointer image = DynamicData::New();
    DummyStreamer::pointer streamer = DummyStreamer::New();
    DummyProcessObject::pointer PO = DummyProcessObject::New();
    streamer->setStreamingMode(STREAMING_MODE_NEWEST_FRAME_ONLY);
    image->setStreamer(streamer);

    Image::pointer frame1 = Image::New();
    Image::pointer frame2 = Image::New();
    Image::pointer frame3 = Image::New();
    image->addFrame(frame1);
    CHECK(image->getNextFrame(PO) == frame1);
    image->addFrame(frame2);
    image->addFrame(frame3);
    CHECK(image->getNextFrame(PO) == frame3);
    CHECK(image->getNextFrame(PO) == frame3);
}

TEST_CASE("Dynamic image with streaming mode PROCESS_ALL returns the frames in the order they were added", "[fast][DynamicData]") {
    DynamicData::pointer image = DynamicData::New();
    DummyStreamer::pointer streamer = DummyStreamer::New();
    DummyProcessObject::pointer PO = DummyProcessObject::New();
    streamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
    image->setStreamer(streamer);

    Image::pointer frame1 = Image::New();
    Image::pointer frame2 = Image::New();
    Image::pointer frame3 = Image::New();
    image->addFrame(frame1);
    image->addFrame(frame2);
    image->addFrame(frame3);
    CHECK(image->getNextFrame(PO) == frame1);
    CHECK(image->getNextFrame(PO) == frame2);
    CHECK(image->getNextFrame(PO) == frame3);
}

TEST_CASE("Dynamic image with streaming mode STORE_ALL returns the frames in the order they were added", "[fast][DynamicData]") {
    DynamicData::pointer image = DynamicData::New();
    DummyStreamer::pointer streamer = DummyStreamer::New();
    DummyProcessObject::pointer PO = DummyProcessObject::New();
    streamer->setStreamingMode(STREAMING_MODE_STORE_ALL_FRAMES);
    image->setStreamer(streamer);

    Image::pointer frame1 = Image::New();
    Image::pointer frame2 = Image::New();
    Image::pointer frame3 = Image::New();
    image->addFrame(frame1);
    image->addFrame(frame2);
    image->addFrame(frame3);
    CHECK(image->getNextFrame(PO) == frame1);
    CHECK(image->getNextFrame(PO) == frame2);
    CHECK(image->getNextFrame(PO) == frame3);
}

TEST_CASE("Dynamic image with streaming mode PROCESS_ALL throws exception when all frames have been retrieved", "[fast][DynamicData]") {
    DynamicData::pointer image = DynamicData::New();
    DummyStreamer::pointer streamer = DummyStreamer::New();
    DummyProcessObject::pointer PO = DummyProcessObject::New();
    streamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
    image->setStreamer(streamer);

    Image::pointer frame1 = Image::New();
    Image::pointer frame2 = Image::New();
    Image::pointer frame3 = Image::New();
    image->addFrame(frame1);
    image->addFrame(frame2);
    image->addFrame(frame3);
    image->getNextFrame(PO);
    image->getNextFrame(PO);
    image->getNextFrame(PO);
    CHECK_THROWS(image->getNextFrame(PO));
}

TEST_CASE("Dynamic image with streaming mode STORE_ALL throws exception when all frames have been retrieved", "[fast][DynamicData]") {
    DynamicData::pointer image = DynamicData::New();
    DummyStreamer::pointer streamer = DummyStreamer::New();
    DummyProcessObject::pointer PO = DummyProcessObject::New();
    streamer->setStreamingMode(STREAMING_MODE_STORE_ALL_FRAMES);
    image->setStreamer(streamer);

    Image::pointer frame1 = Image::New();
    Image::pointer frame2 = Image::New();
    Image::pointer frame3 = Image::New();
    image->addFrame(frame1);
    image->addFrame(frame2);
    image->addFrame(frame3);
    image->getNextFrame(PO);
    image->getNextFrame(PO);
    image->getNextFrame(PO);
    CHECK_THROWS(image->getNextFrame(PO));
}

TEST_CASE("Dynamic image with streaming mode NEWEST_FRAME_ONLY does not update timestamp after getNextFrame is called", "[fast][DynamicData]") {
    DynamicData::pointer image = DynamicData::New();
    DummyStreamer::pointer streamer = DummyStreamer::New();
    DummyProcessObject::pointer PO = DummyProcessObject::New();
    streamer->setStreamingMode(STREAMING_MODE_NEWEST_FRAME_ONLY);
    image->setStreamer(streamer);

    Image::pointer frame1 = Image::New();
    Image::pointer frame2 = Image::New();
    image->addFrame(frame1);
    image->addFrame(frame2);
    unsigned long timestamp = image->getTimestamp();
    image->getNextFrame(PO);
    CHECK(image->getTimestamp() == timestamp);
    image->getNextFrame(PO);
    CHECK(image->getTimestamp() == timestamp);
}

TEST_CASE("Dynamic image with streaming mode PROCESS_ALL does update timestamp after getNextFrame is called and there is more than one frame left", "[fast][DynamicData]") {
    DynamicData::pointer image = DynamicData::New();
    DummyStreamer::pointer streamer = DummyStreamer::New();
    DummyProcessObject::pointer PO = DummyProcessObject::New();
    streamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
    image->setStreamer(streamer);

    Image::pointer frame1 = Image::New();
    Image::pointer frame2 = Image::New();
    Image::pointer frame3 = Image::New();
    image->addFrame(frame1);
    image->addFrame(frame2);
    image->addFrame(frame3);
    unsigned long timestamp = image->getTimestamp();
    image->getNextFrame(PO);
    CHECK(image->getTimestamp() != timestamp);
    timestamp = image->getTimestamp();
    image->getNextFrame(PO);
    CHECK(image->getTimestamp() != timestamp);
    timestamp = image->getTimestamp();
    image->getNextFrame(PO);
    CHECK(image->getTimestamp() == timestamp);
}

TEST_CASE("Dynamic image with streaming mode STORE_ALL does update timestamp after getNextFrame is called and it has not reached the end", "[fast][DynamicData]") {
    DynamicData::pointer image = DynamicData::New();
    DummyStreamer::pointer streamer = DummyStreamer::New();
    DummyProcessObject::pointer PO = DummyProcessObject::New();
    streamer->setStreamingMode(STREAMING_MODE_STORE_ALL_FRAMES);
    image->setStreamer(streamer);

    Image::pointer frame1 = Image::New();
    Image::pointer frame2 = Image::New();
    Image::pointer frame3 = Image::New();
    image->addFrame(frame1);
    image->addFrame(frame2);
    image->addFrame(frame3);
    unsigned long timestamp = image->getTimestamp();
    image->getNextFrame(PO);
    CHECK(image->getTimestamp() != timestamp);
    timestamp = image->getTimestamp();
    image->getNextFrame(PO);
    CHECK(image->getTimestamp() != timestamp);
    timestamp = image->getTimestamp();
    image->getNextFrame(PO);
    //CHECK(image->getTimestamp() == timestamp); TODO
}

TEST_CASE("Dynamic image with streaming mode NEWEST_FRAME_ONLY is marked as has reached end when streamer is marked as has reached end", "[fast][DynamicData]") {
    DynamicData::pointer image = DynamicData::New();
    DummyStreamer::pointer streamer = DummyStreamer::New();
    streamer->setStreamingMode(STREAMING_MODE_NEWEST_FRAME_ONLY);
    image->setStreamer(streamer);

    CHECK(image->hasReachedEnd() == false);
    streamer->setReachedEnd();
    CHECK(image->hasReachedEnd() == true);
}

TEST_CASE("Dynamic image with streaming mode PROCESS_ALL is marked as has reached end when streamer is marked as has reached end and there are no more frames left", "[fast][DynamicData]") {
    {
        DynamicData::pointer image = DynamicData::New();
        DummyStreamer::pointer streamer = DummyStreamer::New();
        DummyProcessObject::pointer PO = DummyProcessObject::New();
        streamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
        image->setStreamer(streamer);

        Image::pointer frame1 = Image::New();
        Image::pointer frame2 = Image::New();
        image->addFrame(frame1);
        image->addFrame(frame2);
        CHECK(image->hasReachedEnd() == false);
        streamer->setReachedEnd();
        CHECK(image->hasReachedEnd() == false);
        image->getNextFrame(PO);
        CHECK(image->hasReachedEnd() == false);
        image->getNextFrame(PO);
        CHECK(image->hasReachedEnd() == true);
    }
    {
        DynamicData::pointer image = DynamicData::New();
        DummyStreamer::pointer streamer = DummyStreamer::New();
        DummyProcessObject::pointer PO = DummyProcessObject::New();
        streamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
        image->setStreamer(streamer);

        Image::pointer frame1 = Image::New();
        Image::pointer frame2 = Image::New();
        image->addFrame(frame1);
        image->addFrame(frame2);
        CHECK(image->hasReachedEnd() == false);
        image->getNextFrame(PO);
        CHECK(image->hasReachedEnd() == false);
        image->getNextFrame(PO);
        CHECK(image->hasReachedEnd() == false);
        streamer->setReachedEnd();
        CHECK(image->hasReachedEnd() == true);
    }
}

TEST_CASE("Dynamic image with streaming mode STORE_ALL is marked as has reached end when streamer is marked as has reached end and there are no more frames left", "[fast][DynamicData]") {
    {
        DynamicData::pointer image = DynamicData::New();
        DummyStreamer::pointer streamer = DummyStreamer::New();
        DummyProcessObject::pointer PO = DummyProcessObject::New();
        streamer->setStreamingMode(STREAMING_MODE_STORE_ALL_FRAMES);
        image->setStreamer(streamer);

        Image::pointer frame1 = Image::New();
        Image::pointer frame2 = Image::New();
        image->addFrame(frame1);
        image->addFrame(frame2);
        CHECK(image->hasReachedEnd() == false);
        streamer->setNrOfFrames(2);
        streamer->setReachedEnd();
        CHECK(image->hasReachedEnd() == false);
        image->getNextFrame(PO);
        CHECK(image->hasReachedEnd() == false);
        image->getNextFrame(PO);
        CHECK(image->hasReachedEnd() == true);
    }
    {
        DynamicData::pointer image = DynamicData::New();
        DummyStreamer::pointer streamer = DummyStreamer::New();
        DummyProcessObject::pointer PO = DummyProcessObject::New();
        streamer->setStreamingMode(STREAMING_MODE_STORE_ALL_FRAMES);
        image->setStreamer(streamer);

        Image::pointer frame1 = Image::New();
        Image::pointer frame2 = Image::New();
        image->addFrame(frame1);
        image->addFrame(frame2);
        streamer->setNrOfFrames(2);
        CHECK(image->hasReachedEnd() == false);
        image->getNextFrame(PO);
        CHECK(image->hasReachedEnd() == false);
        image->getNextFrame(PO);
        CHECK(image->hasReachedEnd() == false);
        streamer->setReachedEnd();
        CHECK(image->hasReachedEnd() == true);
    }
}

TEST_CASE("DynamicData with more than one consumer object returns correct frame with PROCESS_ALL", "[fast][DynamicData]") {
    DynamicData::pointer image = DynamicData::New();
    DummyStreamer::pointer streamer = DummyStreamer::New();
    streamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
    image->setStreamer(streamer);

    Image::pointer frame1 = Image::New();
    Image::pointer frame2 = Image::New();
    Image::pointer frame3 = Image::New();
    image->addFrame(frame1);
    image->addFrame(frame2);
    image->addFrame(frame3);

    DummyProcessObject::pointer PO1 = DummyProcessObject::New();
    DummyProcessObject::pointer PO2 = DummyProcessObject::New();
    image->registerConsumer(PO1);
    image->registerConsumer(PO2);
    CHECK(image->getSize() == 3);
    Image::pointer frame1PO1 = image->getNextFrame(PO1); // frame nr for PO1 is set to 0
    CHECK(image->getSize() == 3);
    Image::pointer frame1PO2 = image->getNextFrame(PO2); // frame nr for PO2 is set to 0
    CHECK(image->getSize() == 2);
    Image::pointer frame2PO1 = image->getNextFrame(PO1); // frame nr for PO1 is set to 1
    CHECK(image->getSize() == 2);
    Image::pointer frame2PO2 = image->getNextFrame(PO2); // frame nr for PO2 is set to 1, frame 0 should be deleted here
    CHECK(image->getSize() == 1);
    Image::pointer frame3PO1 = image->getNextFrame(PO1); // frame nr for PO1 is set to 2
    CHECK(image->getSize() == 1);
    Image::pointer frame3PO2 = image->getNextFrame(PO2); // frame nr for PO2 is set to 2, frame 1 should be deleted here
    CHECK(image->getSize() == 0);

    CHECK(frame1 == frame1PO1);
    CHECK(frame1 == frame1PO2);
    CHECK(frame2 == frame2PO1);
    CHECK(frame2 == frame2PO2);
    CHECK(frame3 == frame3PO1);
    CHECK(frame3 == frame3PO2);
}

TEST_CASE("DynamicData with more than one consumer object returns correct frame with PROCESS_ALL and maximum nr of frames is set", "[fast][DynamicData]") {
    DynamicData::pointer image = DynamicData::New();
    DummyStreamer::pointer streamer = DummyStreamer::New();
    streamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
    image->setStreamer(streamer);
    image->setMaximumNumberOfFrames(1000);

    Image::pointer frame1 = Image::New();
    Image::pointer frame2 = Image::New();
    Image::pointer frame3 = Image::New();
    image->addFrame(frame1);
    image->addFrame(frame2);
    image->addFrame(frame3);

    DummyProcessObject::pointer PO1 = DummyProcessObject::New();
    DummyProcessObject::pointer PO2 = DummyProcessObject::New();
    image->registerConsumer(PO1);
    image->registerConsumer(PO2);
    CHECK(image->getSize() == 3);
    Image::pointer frame1PO1 = image->getNextFrame(PO1); // frame nr for PO1 is set to 0
    CHECK(image->getSize() == 3);
    Image::pointer frame1PO2 = image->getNextFrame(PO2); // frame nr for PO2 is set to 0
    CHECK(image->getSize() == 2);
    Image::pointer frame2PO1 = image->getNextFrame(PO1); // frame nr for PO1 is set to 1
    CHECK(image->getSize() == 2);
    Image::pointer frame2PO2 = image->getNextFrame(PO2); // frame nr for PO2 is set to 1, frame 0 should be deleted here
    CHECK(image->getSize() == 1);
    Image::pointer frame3PO1 = image->getNextFrame(PO1); // frame nr for PO1 is set to 2
    CHECK(image->getSize() == 1);
    Image::pointer frame3PO2 = image->getNextFrame(PO2); // frame nr for PO2 is set to 2, frame 1 should be deleted here
    CHECK(image->getSize() == 0);

    CHECK(frame1 == frame1PO1);
    CHECK(frame1 == frame1PO2);
    CHECK(frame2 == frame2PO1);
    CHECK(frame2 == frame2PO2);
    CHECK(frame3 == frame3PO1);
    CHECK(frame3 == frame3PO2);
}

TEST_CASE("DynamicData with more than one consumer object returns correct frame with STORE_ALL", "[fast][DynamicData]") {
    DynamicData::pointer image = DynamicData::New();
    DummyStreamer::pointer streamer = DummyStreamer::New();
    streamer->setStreamingMode(STREAMING_MODE_STORE_ALL_FRAMES);
    image->setStreamer(streamer);

    Image::pointer frame1 = Image::New();
    Image::pointer frame2 = Image::New();
    image->addFrame(frame1);
    image->addFrame(frame2);

    DummyProcessObject::pointer PO1 = DummyProcessObject::New();
    DummyProcessObject::pointer PO2 = DummyProcessObject::New();
    CHECK(image->getSize() == 2);
    Image::pointer frame1PO1 = image->getNextFrame(PO1);
    CHECK(image->getSize() == 2);
    Image::pointer frame1PO2 = image->getNextFrame(PO2);
    CHECK(image->getSize() == 2);
    Image::pointer frame2PO1 = image->getNextFrame(PO1);
    CHECK(image->getSize() == 2);
    Image::pointer frame2PO2 = image->getNextFrame(PO2);
    CHECK(image->getSize() == 2);

    CHECK(frame1 == frame1PO1);
    CHECK(frame1 == frame1PO2);
    CHECK(frame2 == frame2PO1);
    CHECK(frame2 == frame2PO2);
}

TEST_CASE("DynamicData with more than one consumer object returns correct frame with NEWEST_ONLY", "[fast][DynamicData]") {
    DynamicData::pointer image = DynamicData::New();
    DummyStreamer::pointer streamer = DummyStreamer::New();
    streamer->setStreamingMode(STREAMING_MODE_NEWEST_FRAME_ONLY);
    image->setStreamer(streamer);

    Image::pointer frame1 = Image::New();
    Image::pointer frame2 = Image::New();
    image->addFrame(frame1);
    image->addFrame(frame2);

    DummyProcessObject::pointer PO1 = DummyProcessObject::New();
    DummyProcessObject::pointer PO2 = DummyProcessObject::New();
    CHECK(image->getSize() == 1);
    Image::pointer frame1PO1 = image->getNextFrame(PO1);
    CHECK(image->getSize() == 1);
    Image::pointer frame1PO2 = image->getNextFrame(PO2);
    CHECK(image->getSize() == 1);
    Image::pointer frame2PO1 = image->getNextFrame(PO1);
    CHECK(image->getSize() == 1);
    Image::pointer frame2PO2 = image->getNextFrame(PO2);
    CHECK(image->getSize() == 1);

    CHECK(frame2 == frame1PO1);
    CHECK(frame2 == frame1PO2);
    CHECK(frame2 == frame2PO1);
    CHECK(frame2 == frame2PO2);
}


