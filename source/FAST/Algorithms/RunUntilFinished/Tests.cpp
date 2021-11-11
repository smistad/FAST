#include <FAST/Testing.hpp>
#include "RunUntilFinished.hpp"
#include <FAST/Tests/DummyObjects.hpp>

using namespace fast;

TEST_CASE("Run until finished", "[fast][RunUntilFinished]") {
    auto streamer = DummyStreamer::New();
    streamer->setTotalFrames(10);

    auto runUntilFinished = RunUntilFinished::create()
            ->connect(streamer);
    auto data = runUntilFinished->runAndGetOutputData<DummyDataObject>();
    CHECK(data->getID() == 9);
}