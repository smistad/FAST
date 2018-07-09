#include "catch.hpp"
#include "DummyObjects.hpp"

namespace fast {

// Stream data only tests
TEST_CASE("Simple pipeline with stream", "[process_all_frames][ProcessObject][fast]") {
    DummyStreamer::pointer streamer = DummyStreamer::New();
    streamer->setSleepTime(10);
    streamer->setTotalFrames(20);

    DummyProcessObject::pointer po = DummyProcessObject::New();
    po->setInputConnection(streamer->getOutputPort());

    DataPort::pointer port = po->getOutputPort();
    port->setMaximumNumberOfFrames(1);

    int timestep = 0;
    while(port->getFrameCounter() != streamer->getFramesToGenerate()) {
        po->update(timestep);
        DummyDataObject::pointer image = port->getNextFrame<DummyDataObject>();
        CHECK(image->getID() == timestep);
        timestep++;
    }
    CHECK(timestep == 20);
}

TEST_CASE("Two step pipeline with stream", "[two_step][process_all_frames][ProcessObject][fast]") {
    DummyStreamer::pointer streamer = DummyStreamer::New();
    streamer->setSleepTime(10);
    streamer->setTotalFrames(20);

    DummyProcessObject::pointer po1 = DummyProcessObject::New();
    po1->setInputConnection(streamer->getOutputPort());

    DummyProcessObject::pointer po2 = DummyProcessObject::New();
    po2->setInputConnection(po1->getOutputPort());

    DataPort::pointer port = po2->getOutputPort();

    int timestep = 0;
    while(port->getFrameCounter() != streamer->getFramesToGenerate()) {
        po2->update(timestep);
        DummyDataObject::pointer image = port->getNextFrame<DummyDataObject>();
        CHECK(image->getID() == timestep);
        timestep++;
    }
    CHECK(timestep == 20);
}

TEST_CASE("Simple pipeline with stream, NEWEST_FRAME_ONLY", "[ProcessObject][fast]") {
    DummyStreamer::pointer streamer = DummyStreamer::New();
    streamer->setSleepTime(100);
    streamer->setTotalFrames(20);

    DummyProcessObject::pointer po = DummyProcessObject::New();
    po->setInputConnection(streamer->getOutputPort());

    DataPort::pointer port = po->getOutputPort();

    int timestep = 0;
    int previousID = -1;
    while(!streamer->hasReachedEnd()) {
        po->update(timestep, STREAMING_MODE_NEWEST_FRAME_ONLY);
        timestep++;
        DummyDataObject::pointer image = port->getNextFrame<DummyDataObject>();
        CHECK(image->getID() != previousID); // Should never get the same frame
        previousID = image->getID();
    }
}

TEST_CASE("Two step pipeline with stream, NEWEST_FRAME_ONLY", "[two_step][ProcessObject][fast]") {
    DummyStreamer::pointer streamer = DummyStreamer::New();
    streamer->setSleepTime(100);
    streamer->setTotalFrames(20);

    DummyProcessObject::pointer po1 = DummyProcessObject::New();
    po1->setInputConnection(streamer->getOutputPort());

    DummyProcessObject::pointer po2 = DummyProcessObject::New();
    po2->setInputConnection(po1->getOutputPort());

    DataPort::pointer port = po2->getOutputPort();

    int timestep = 0;
    int previousID = -1;
    while(!streamer->hasReachedEnd()) {
        po2->update(timestep, STREAMING_MODE_NEWEST_FRAME_ONLY);
        timestep++;
        DummyDataObject::pointer image = port->getNextFrame<DummyDataObject>();
        CHECK(image->getID() != previousID); // Should never get the same frame
        previousID = image->getID();
    }
}


TEST_CASE("Simple pipeline with stream, STORE_ALL", "[ProcessObject][fast]") {
    int frames = 20;
    DummyStreamer::pointer streamer = DummyStreamer::New();
    streamer->setSleepTime(10);
    streamer->setTotalFrames(frames);

    DummyProcessObject::pointer po = DummyProcessObject::New();
    po->setInputConnection(streamer->getOutputPort());

    DataPort::pointer port = po->getOutputPort();

    for(int i = 0; i < 3; i++) { // Loop over data stream 3 times
        std::cout << "NEW ROUND IN STORE ALL TEST" << std::endl;
        int timestep = 0;
        int previousID = -1;
        while(timestep < frames) {
            po->update(timestep, STREAMING_MODE_STORE_ALL_FRAMES);
            timestep++;
            DummyDataObject::pointer image = port->getNextFrame<DummyDataObject>();
            CHECK(image->getID() != previousID); // Should never get the same frame
            previousID = image->getID();
        }
    }
}

TEST_CASE("Two step pipeline with stream, STORE_ALL", "[two_step][ProcessObject][fast]") {
    int frames = 20;
    DummyStreamer::pointer streamer = DummyStreamer::New();
    streamer->setSleepTime(10);
    streamer->setTotalFrames(frames);

    DummyProcessObject::pointer po1 = DummyProcessObject::New();
    po1->setInputConnection(streamer->getOutputPort());

    DummyProcessObject::pointer po2 = DummyProcessObject::New();
    po2->setInputConnection(po1->getOutputPort());

    DataPort::pointer port = po2->getOutputPort();

    for(int i = 0; i < 3; i++) { // Loop over data stream 3 times
        std::cout << "NEW ROUND IN STORE ALL TEST" << std::endl;
        int timestep = 0;
        int previousID = -1;
        while(timestep < frames) {
            po2->update(timestep, STREAMING_MODE_STORE_ALL_FRAMES);
            timestep++;
            DummyDataObject::pointer image = port->getNextFrame<DummyDataObject>();
            CHECK(image->getID() != previousID); // Should never get the same frame
            previousID = image->getID();
        }
    }
}

// Static data only tests
TEST_CASE("Simple pipeline with static data", "[process_all_frames][ProcessObject][fast]") {
    DummyImporter::pointer importer = DummyImporter::New();

    DummyProcessObject::pointer po1 = DummyProcessObject::New();
    po1->setInputConnection(importer->getOutputPort());

    DataPort::pointer port = po1->getOutputPort();

    int timestep = 0;
    po1->update(timestep, STREAMING_MODE_PROCESS_ALL_FRAMES);
    CHECK(po1->hasExecuted());
    po1->setHasExecuted(false);

    while(timestep < 3) {
        timestep++;
        po1->update(timestep, STREAMING_MODE_PROCESS_ALL_FRAMES);
        CHECK_FALSE(po1->hasExecuted()); // PO1 should not re execute
        po1->setHasExecuted(false);
        DummyDataObject::pointer image = port->getNextFrame<DummyDataObject>();
        CHECK(image->getID() == 0); // Should always get the same frame
    }

    importer->setModified();
    po1->update(timestep, STREAMING_MODE_PROCESS_ALL_FRAMES);
    CHECK(po1->hasExecuted()); // PO should execute after importer has been modified
    po1->setHasExecuted(false);

    while(timestep < 6) {
        timestep++;
        po1->update(timestep, STREAMING_MODE_PROCESS_ALL_FRAMES);
        CHECK_FALSE(po1->hasExecuted()); // PO1 should not re execute
        po1->setHasExecuted(false);
        DummyDataObject::pointer image = port->getNextFrame<DummyDataObject>();
        CHECK(image->getID() == 1); // Should always get the same frame
    }
}

TEST_CASE("Simple pipeline with static data, NEWEST_FRAME", "[ProcessObject][fast]") {
    DummyImporter::pointer importer = DummyImporter::New();

    DummyProcessObject::pointer po1 = DummyProcessObject::New();
    po1->setInputConnection(importer->getOutputPort());

    DataPort::pointer port = po1->getOutputPort();

    int timestep = 0;
    while(timestep < 3) {
        po1->update(timestep, STREAMING_MODE_NEWEST_FRAME_ONLY);
        timestep++;
        DummyDataObject::pointer image = port->getNextFrame<DummyDataObject>();
        CHECK(image->getID() == 0); // Should always get the same frame
    }

    importer->setModified();
    //timestep = 0;
    while(timestep < 6) {
        po1->update(timestep, STREAMING_MODE_NEWEST_FRAME_ONLY);
        timestep++;
        DummyDataObject::pointer image = port->getNextFrame<DummyDataObject>();
        CHECK(image->getID() == 1); // Should always get the same frame
    }
}

TEST_CASE("Simple pipeline with static data, STORE_ALL", "[ProcessObject][fast]") {
    DummyImporter::pointer importer = DummyImporter::New();

    DummyProcessObject::pointer po1 = DummyProcessObject::New();
    po1->setInputConnection(importer->getOutputPort());

    DataPort::pointer port = po1->getOutputPort();

    int timestep = 0;
    while(timestep < 3) {
        po1->update(timestep, STREAMING_MODE_STORE_ALL_FRAMES);
        timestep++;
        DummyDataObject::pointer image = port->getNextFrame<DummyDataObject>();
        CHECK(image->getID() == 0); // Should always get the same frame
    }

    importer->setModified();
    //timestep = 0;
    while(timestep < 6) {
        po1->update(timestep, STREAMING_MODE_STORE_ALL_FRAMES);
        timestep++;
        DummyDataObject::pointer image = port->getNextFrame<DummyDataObject>();
        CHECK(image->getID() == 1); // Should always get the same frame
    }
}

// Static + stream data tests
TEST_CASE("Simple pipeline with static and stream data, PROCESS_ALL", "[process_all_frames][static_and_stream][ProcessObject][fast]") {

    int frames = 10;
    DummyStreamer::pointer streamer = DummyStreamer::New();
    streamer->setSleepTime(10);
    streamer->setTotalFrames(frames);

    DummyImporter::pointer importer = DummyImporter::New();

    DummyProcessObject2::pointer po1 = DummyProcessObject2::New();
    po1->setInputConnection(0, streamer->getOutputPort());
    po1->setInputConnection(1, importer->getOutputPort());

    DataPort::pointer port = po1->getOutputPort();

    int timestep = 0;
    while(timestep < frames / 2) {
        po1->update(timestep, STREAMING_MODE_PROCESS_ALL_FRAMES);
        DummyDataObject::pointer image = port->getNextFrame<DummyDataObject>();
        CHECK(image->getID() == timestep);
        CHECK(po1->getStaticDataID() == 0);
        timestep++;
    }

    importer->setModified();
    while(timestep < frames) {
        po1->update(timestep, STREAMING_MODE_PROCESS_ALL_FRAMES);
        DummyDataObject::pointer image = port->getNextFrame<DummyDataObject>();
        CHECK(image->getID() == timestep);
        CHECK(po1->getStaticDataID() == 1);
        timestep++;
    }
}

TEST_CASE("Static data with multiple receiver POs, PROCESS_ALL", "[process_all_frames][ProcessObject][fast]") {
    const int frames = 4;
    DummyImporter::pointer importer = DummyImporter::New();

    DummyProcessObject::pointer po1 = DummyProcessObject::New();
    po1->setInputConnection(importer->getOutputPort());
    DummyProcessObject::pointer po2 = DummyProcessObject::New();
    po2->setInputConnection(importer->getOutputPort());
    DummyProcessObject::pointer po3 = DummyProcessObject::New();
    po3->setInputConnection(po2->getOutputPort());

    DataPort::pointer port1 = po1->getOutputPort();
    DataPort::pointer port2 = po3->getOutputPort();

    int timestep = 0;
    while(timestep < frames) {
        // Update both branches
        po1->update(timestep, STREAMING_MODE_PROCESS_ALL_FRAMES);
        po3->update(timestep, STREAMING_MODE_PROCESS_ALL_FRAMES);

        DummyDataObject::pointer image1 = port1->getNextFrame<DummyDataObject>();
        DummyDataObject::pointer image2 = port2->getNextFrame<DummyDataObject>();

        CHECK(image1->getID() == 0);
        CHECK(image2->getID() == 0);
        timestep++;
    }
}

TEST_CASE("Static data with multiple receiver POs, NEWEST_FRAME", "[ProcessObject][fast]") {
    const int frames = 4;
    DummyImporter::pointer importer = DummyImporter::New();

    DummyProcessObject::pointer po1 = DummyProcessObject::New();
    po1->setInputConnection(importer->getOutputPort());
    DummyProcessObject::pointer po2 = DummyProcessObject::New();
    po2->setInputConnection(importer->getOutputPort());
    DummyProcessObject::pointer po3 = DummyProcessObject::New();
    po3->setInputConnection(po2->getOutputPort());

    DataPort::pointer port1 = po1->getOutputPort();
    DataPort::pointer port2 = po3->getOutputPort();

    int timestep = 0;
    while(timestep < frames) {
        // Update both branches
        po1->update(timestep, STREAMING_MODE_NEWEST_FRAME_ONLY);
        po3->update(timestep, STREAMING_MODE_NEWEST_FRAME_ONLY);

        DummyDataObject::pointer image1 = port1->getNextFrame<DummyDataObject>();
        DummyDataObject::pointer image2 = port2->getNextFrame<DummyDataObject>();

        CHECK(image1->getID() == 0);
        CHECK(image2->getID() == 0);
        timestep++;
    }
}


TEST_CASE("Static data with multiple receiver POs, STORE_ALL", "[ProcessObject][fast][asdasd2]") {
    const int frames = 4;
    DummyImporter::pointer importer = DummyImporter::New();

    DummyProcessObject::pointer po1 = DummyProcessObject::New();
    po1->setInputConnection(importer->getOutputPort());
    DummyProcessObject::pointer po2 = DummyProcessObject::New();
    po2->setInputConnection(importer->getOutputPort());
    DummyProcessObject::pointer po3 = DummyProcessObject::New();
    po3->setInputConnection(po2->getOutputPort());

    DataPort::pointer port1 = po1->getOutputPort();
    DataPort::pointer port2 = po3->getOutputPort();

    int timestep = 0;
    while(timestep < frames) {
        std::cout << "TIMESTEP " << timestep << std::endl;
        // Update both branches
        po1->update(timestep, STREAMING_MODE_STORE_ALL_FRAMES);
        po3->update(timestep, STREAMING_MODE_STORE_ALL_FRAMES);

        DummyDataObject::pointer image1 = port1->getNextFrame<DummyDataObject>();
        DummyDataObject::pointer image2 = port2->getNextFrame<DummyDataObject>();

        CHECK(image1->getID() == 0);
        CHECK(image2->getID() == 0);
        timestep++;
    }
}

TEST_CASE("Stream with multiple receiver POs, PROCESS_ALL", "[process_all_frames][ProcessObject][fast]") {
    const int frames = 20;
    DummyStreamer::pointer streamer = DummyStreamer::New();
    streamer->setSleepTime(10);
    streamer->setTotalFrames(frames);

    DummyProcessObject::pointer po1 = DummyProcessObject::New();
    po1->setInputConnection(streamer->getOutputPort());
    DummyProcessObject::pointer po2 = DummyProcessObject::New();
    po2->setInputConnection(streamer->getOutputPort());
    DummyProcessObject::pointer po3 = DummyProcessObject::New();
    po3->setInputConnection(po2->getOutputPort());

    DataPort::pointer port1 = po1->getOutputPort();
    DataPort::pointer port2 = po3->getOutputPort();

    int timestep = 0;
    while(timestep < frames) {
        // Update both branches
        po1->update(timestep, STREAMING_MODE_PROCESS_ALL_FRAMES);
        po3->update(timestep, STREAMING_MODE_PROCESS_ALL_FRAMES);

        DummyDataObject::pointer image1 = port1->getNextFrame<DummyDataObject>();
        DummyDataObject::pointer image2 = port2->getNextFrame<DummyDataObject>();

        CHECK(image1->getID() == timestep);
        CHECK(image2->getID() == timestep);
        timestep++;
    }
}


TEST_CASE("Stream with multiple receiver POs, NEWEST_FRAME", "[ProcessObject][fast]") {
    const int frames = 20;
    DummyStreamer::pointer streamer = DummyStreamer::New();
    streamer->setSleepTime(10);
    streamer->setTotalFrames(frames);

    DummyProcessObject::pointer po1 = DummyProcessObject::New();
    po1->setInputConnection(streamer->getOutputPort());
    DummyProcessObject::pointer po2 = DummyProcessObject::New();
    po2->setInputConnection(streamer->getOutputPort());
    DummyProcessObject::pointer po3 = DummyProcessObject::New();
    po3->setInputConnection(po2->getOutputPort());

    DataPort::pointer port1 = po1->getOutputPort();
    DataPort::pointer port2 = po3->getOutputPort();

    int timestep = 0;
    int previousID1 = -1;
    int previousID2 = -1;
    while(timestep < frames) {
        // Update both branches
        po1->update(timestep, STREAMING_MODE_NEWEST_FRAME_ONLY);
        po3->update(timestep, STREAMING_MODE_NEWEST_FRAME_ONLY);

        DummyDataObject::pointer image1 = port1->getNextFrame<DummyDataObject>();
        DummyDataObject::pointer image2 = port2->getNextFrame<DummyDataObject>();

        // Should never process the same image twice
        CHECK(image1->getID() != previousID1);
        CHECK(image2->getID() != previousID2);
        previousID1 = image1->getID();
        previousID2 = image2->getID();
        timestep++;
    }
}

TEST_CASE("Stream with multiple receiver POs, STORE_ALL", "[ProcessObject][fast]") {
    const int frames = 20;
    DummyStreamer::pointer streamer = DummyStreamer::New();
    streamer->setSleepTime(10);
    streamer->setTotalFrames(frames);

    DummyProcessObject::pointer po1 = DummyProcessObject::New();
    po1->setInputConnection(streamer->getOutputPort());
    DummyProcessObject::pointer po2 = DummyProcessObject::New();
    po2->setInputConnection(streamer->getOutputPort());
    DummyProcessObject::pointer po3 = DummyProcessObject::New();
    po3->setInputConnection(po2->getOutputPort());

    DataPort::pointer port1 = po1->getOutputPort();
    DataPort::pointer port2 = po3->getOutputPort();

    for(int i = 0; i < 3; i++) {
        int timestep = 0;
        while(timestep < frames) {
            // Update both branches
            po1->update(timestep, STREAMING_MODE_STORE_ALL_FRAMES);
            po3->update(timestep, STREAMING_MODE_STORE_ALL_FRAMES);

            DummyDataObject::pointer image1 = port1->getNextFrame<DummyDataObject>();
            DummyDataObject::pointer image2 = port2->getNextFrame<DummyDataObject>();

            // Should never process the same image twice
            CHECK(image1->getID() == timestep);
            CHECK(image2->getID() == timestep);
            timestep++;
        }
    }
}

// Static data only tests using setInputData
TEST_CASE("Simple pipeline with static data using setInputData", "[process_all_frames][ProcessObject][fast]") {
    DummyDataObject::pointer image = DummyDataObject::New();
    image->create(0);

    DummyProcessObject::pointer po1 = DummyProcessObject::New();
    po1->setInputData(image);

    DataPort::pointer port = po1->getOutputPort();

    int timestep = 0;
    while(timestep < 3) {
        po1->update(timestep, STREAMING_MODE_PROCESS_ALL_FRAMES);
        timestep++;
        DummyDataObject::pointer image = port->getNextFrame<DummyDataObject>();
        CHECK(image->getID() == 0); // Should always get the same frame
    }
}

TEST_CASE("Simple pipeline with static data using setInputData, NEWEST_FRAME", "[ProcessObject][fast]") {
    DummyDataObject::pointer image = DummyDataObject::New();
    image->create(0);

    DummyProcessObject::pointer po1 = DummyProcessObject::New();
    po1->setInputData(image);

    DataPort::pointer port = po1->getOutputPort();

    int timestep = 0;
    while(timestep < 3) {
        po1->update(timestep, STREAMING_MODE_NEWEST_FRAME_ONLY);
        timestep++;
        DummyDataObject::pointer image = port->getNextFrame<DummyDataObject>();
        CHECK(image->getID() == 0); // Should always get the same frame
    }
}

TEST_CASE("Simple pipeline with static data using setInputData, STORE_ALL", "[ProcessObject][fast]") {
    DummyDataObject::pointer image = DummyDataObject::New();
    image->create(0);

    DummyProcessObject::pointer po1 = DummyProcessObject::New();
    po1->setInputData(image);

    DataPort::pointer port = po1->getOutputPort();

    int timestep = 0;
    while(timestep < 3) {
        po1->update(timestep, STREAMING_MODE_STORE_ALL_FRAMES);
        timestep++;
        DummyDataObject::pointer image = port->getNextFrame<DummyDataObject>();
        CHECK(image->getID() == 0); // Should always get the same frame
    }
}

TEST_CASE("Missing input throws exception on execute", "[ProcessObject][fast]") {
    DummyProcessObject::pointer po = DummyProcessObject::New();
    po->setIsModified();
    CHECK_THROWS(po->update(0));
}

TEST_CASE("Trying to use non existent input and output ports throws exception", "[ProcessObject][fast]") {
    DummyProcessObject::pointer po = DummyProcessObject::New();
    CHECK_THROWS(po->getOutputPort(1));

    DummyProcessObject2::pointer po2 = DummyProcessObject2::New();
    CHECK_THROWS(po->setInputConnection(1, po2->getOutputPort()));

    DummyDataObject::pointer image = DummyDataObject::New();
    CHECK_THROWS(po->setInputData(1, image));
}

TEST_CASE("Trying to set input connection to self throws exception", "[ProcessObject][fast]") {
    DummyProcessObject::pointer po = DummyProcessObject::New();
    CHECK_THROWS(po->setInputConnection(po->getOutputPort()));
}

}
