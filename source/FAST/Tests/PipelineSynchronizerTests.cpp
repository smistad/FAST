#include <FAST/Testing.hpp>
#include <FAST/PipelineSynchronizer.hpp>
#include "DummyObjects.hpp"

using namespace fast;

TEST_CASE("Pipeline synchronizer - two streams at very different rates", "[fast][PipelineSynchronizer]") {
    const int frames = 20;
    auto streamer1 = DummyStreamer::New();
    streamer1->setStreamingMode(StreamingMode::NewestFrameOnly);
    streamer1->setTotalFrames(frames);
    streamer1->setSleepTime(1);

    auto streamer2 = DummyStreamer::New();
    streamer2->setStreamingMode(StreamingMode::NewestFrameOnly);
    streamer2->setTotalFrames(frames);
    streamer2->setSleepTime(2000); // 2 seconds

    auto synchronizer = PipelineSynchronizer::New();
    synchronizer->addInputConnection(streamer1->getOutputPort());
    synchronizer->addInputConnection(streamer2->getOutputPort());

    auto port1 = synchronizer->getOutputPort(0);
    auto port2 = synchronizer->getOutputPort(1);

    synchronizer->update(); // Start streams

    // Get data and check
    auto data1 = port1->getNextFrame<DummyDataObject>();
    auto data2 = port2->getNextFrame<DummyDataObject>();
    CHECK(data1->getID() >= 0);
    CHECK(data2->getID() == 0);

    std::this_thread::sleep_for(std::chrono::duration<double>(1)); // sleep for 1 sec

    // Get data and check
    data1 = port1->getNextFrame<DummyDataObject>();
    data2 = port2->getNextFrame<DummyDataObject>();
    CHECK(data1->getID() >= 0);
    CHECK(data2->getID() == 0); // Data 2 should still return the first one
}