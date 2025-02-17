#include <FAST/Testing.hpp>
#include <FAST/FramerateSynchronizer.hpp>
#include "DummyObjects.hpp"

using namespace fast;

TEST_CASE("Framerate synchronizer - two streams at very different rates", "[fast][FramerateSynchronizer]") {
    const int frames = 20;
    auto streamer1 = DummyStreamer::New();
    streamer1->setStreamingMode(StreamingMode::NewestFrameOnly);
    streamer1->setTotalFrames(frames);
    streamer1->setSleepTime(100);

    auto streamer2 = DummyStreamer::New();
    streamer2->setStreamingMode(StreamingMode::NewestFrameOnly);
    streamer2->setTotalFrames(frames);
    streamer2->setSleepTime(200);

    auto synchronizer = FramerateSynchronizer::create();
    synchronizer->connect(0, streamer1);
    synchronizer->connect(1, streamer2);

    auto port1 = synchronizer->getOutputPort(0);
    auto port2 = synchronizer->getOutputPort(1);

    int previousID1 = -1;
    int previousID2 = -1;
    for(int i = 0; i < 10; ++i) {
        std::cout << "Iteration: " << i << std::endl;
        synchronizer->run();
        auto data1 = port1->getNextFrame<DummyDataObject>();
        auto data2 = port2->getNextFrame<DummyDataObject>();
        //CHECK(data1->getID() == i);
        CHECK(((int)data1->getID() != previousID1 || (int)data2->getID() != previousID2));
        previousID1 = data1->getID();
        previousID2 = data2->getID();
        std::cout << previousID1 << " " << previousID2 << std::endl;
        if(data1->isLastFrame() || data2->isLastFrame())
            break;
    }
}

TEST_CASE("Framerate synchronizer - two streams at very different rates and priority port", "[fast][FramerateSynchronizer]") {
    const int frames = 20;
    auto streamer1 = DummyStreamer::New();
    streamer1->setStreamingMode(StreamingMode::NewestFrameOnly);
    streamer1->setTotalFrames(frames);
    streamer1->setSleepTime(100);

    auto streamer2 = DummyStreamer::New();
    streamer2->setStreamingMode(StreamingMode::NewestFrameOnly);
    streamer2->setTotalFrames(frames);
    streamer2->setSleepTime(200);

    auto synchronizer = FramerateSynchronizer::create(0);
    synchronizer->connect(0, streamer1);
    synchronizer->connect(1, streamer2);

    auto port1 = synchronizer->getOutputPort(0);
    auto port2 = synchronizer->getOutputPort(1);

    int previousID1 = -1;
    int previousID2 = -1;
    for(int i = 0; i < 10; ++i) {
        std::cout << "Iteration: " << i << std::endl;
        synchronizer->run();
        auto data1 = port1->getNextFrame<DummyDataObject>();
        auto data2 = port2->getNextFrame<DummyDataObject>();
        //CHECK(data1->getID() == i);
        CHECK((int)data1->getID() > previousID1);
        CHECK((int)data2->getID() >= previousID2);
        previousID1 = data1->getID();
        previousID2 = data2->getID();
        std::cout << previousID1 << " " << previousID2 << std::endl;
        if(data1->isLastFrame() || data2->isLastFrame())
            break;
    }
}
