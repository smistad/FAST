#include "FAST/Testing.hpp"
#include "FAST/DeviceManager.hpp"
#include <thread>
#include "FAST/Streamers/ImageFileStreamer.hpp"
#include "FAST/Tests/DummyObjects.hpp"
#include "FAST/Data/Image.hpp"

using namespace fast;

TEST_CASE("No filename format given to ImageFileStreamer", "[fast][ImageFileStreamer]") {
    ImageFileStreamer::pointer mhdStreamer = ImageFileStreamer::New();
    CHECK_THROWS(mhdStreamer->update());
}

TEST_CASE("No hash tag in filename format given to ImageFileStreamer", "[fast][ImageFileStreamer]") {
    ImageFileStreamer::pointer mhdStreamer = ImageFileStreamer::New();
    CHECK_THROWS(mhdStreamer->setFilenameFormat("asd"));
}

TEST_CASE("Default streaming mode is NEWEST_FRAME_ONLY", "[fast][ImageFileStreamer]") {
    ImageFileStreamer::pointer mhdStreamer = ImageFileStreamer::New();
    CHECK(mhdStreamer->getStreamingMode() == STREAMING_MODE_NEWEST_FRAME_ONLY);
}

/*
// TODO fix this, will not work because it is another thread that throws the exception
TEST_CASE("Wrong filename format given to ImageFileStreamer", "[fast][ImageFileStreamer]") {
    ImageFileStreamer::pointer mhdStreamer = ImageFileStreamer::New();
    mhdStreamer->setFilenameFormat("asd#asd");
    DynamicData::pointer image = mhdStreamer->getOutput();
    reportInfo() << "asd" << Reporter::end();
    CHECK_THROWS(mhdStreamer->update());
}
*/

TEST_CASE("ImageFileStreamer streaming to host with streaming mode NEWEST set", "[fast][ImageFileStreamer]") {
    DummyProcessObject::pointer PO = DummyProcessObject::New();
    ImageFileStreamer::pointer mhdStreamer = ImageFileStreamer::New();
    mhdStreamer->setFilenameFormat(Config::getTestDataPath()+"US/Ball/US-3Dt_#.mhd");
    mhdStreamer->setStreamingMode(STREAMING_MODE_NEWEST_FRAME_ONLY);
    mhdStreamer->setMainDevice(Host::getInstance());
    CHECK_NOTHROW(
    mhdStreamer->update(); // this starts the streamer
    DynamicData::pointer image = mhdStreamer->getOutputData<Image>(0);
    unsigned long currentTimestamp = image->getTimestamp();
    while(!image->hasReachedEnd()) {
        mhdStreamer->update();
        if(currentTimestamp != image->getTimestamp()) {
            currentTimestamp = image->getTimestamp();
            Image::pointer frame = image->getNextFrame(PO);
        }
        // Must make this thread sleep a little so that streamer will get a chance to import data
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    );
}

TEST_CASE("ImageFileStreamer streaming to host with streaming mode PROCESS_ALL set", "[fast][ImageFileStreamer]") {
    DummyProcessObject::pointer PO = DummyProcessObject::New();
    ImageFileStreamer::pointer mhdStreamer = ImageFileStreamer::New();
    mhdStreamer->setFilenameFormat(Config::getTestDataPath()+"US/Ball/US-3Dt_#.mhd");
    mhdStreamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
    mhdStreamer->setMainDevice(Host::getInstance());
    CHECK_NOTHROW(
    mhdStreamer->update(); // this starts the streamer
    DynamicData::pointer image = mhdStreamer->getOutputData<Image>(0);
    unsigned long currentTimestamp = image->getTimestamp();
    while(!image->hasReachedEnd()) {
        mhdStreamer->update();
        if(currentTimestamp != image->getTimestamp()) {
            currentTimestamp = image->getTimestamp();
            Image::pointer frame = image->getNextFrame(PO);
        }
        // Must make this thread sleep a little so that streamer will get a chance to import data
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    );
}

TEST_CASE("ImageFileStreamer streaming to host with streaming mode STORE_ALL set", "[fast][ImageFileStreamer]") {
    DummyProcessObject::pointer PO = DummyProcessObject::New();
    ImageFileStreamer::pointer mhdStreamer = ImageFileStreamer::New();
    mhdStreamer->setFilenameFormat(Config::getTestDataPath()+"US/Ball/US-3Dt_#.mhd");
    mhdStreamer->setStreamingMode(STREAMING_MODE_STORE_ALL_FRAMES);
    mhdStreamer->setMainDevice(Host::getInstance());
    CHECK_NOTHROW(
    mhdStreamer->update(); // this starts the streamer
    DynamicData::pointer image = mhdStreamer->getOutputData<Image>(0);
    unsigned long currentTimestamp = image->getTimestamp();
    while(!image->hasReachedEnd()) {
        mhdStreamer->update();
        if(currentTimestamp != image->getTimestamp()) {
            currentTimestamp = image->getTimestamp();
            Image::pointer frame = image->getNextFrame(PO);
        }
        // Must make this thread sleep a little so that streamer will get a chance to import data
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    );
}

/*
TEST_CASE("ImageFileStreamer with streaming mode STORE_ALL and maximum number of frames throws when limit is reached", "[fast][ImageFileStreamer]") {
    DummyProcessObject::pointer PO = DummyProcessObject::New();
    ImageFileStreamer::pointer mhdStreamer = ImageFileStreamer::New();
    mhdStreamer->setFilenameFormat(Config::getTestDataPath()+"US/Ball/US-3Dt_#.mhd");
    mhdStreamer->setStreamingMode(STREAMING_MODE_STORE_ALL_FRAMES);
    mhdStreamer->setMaximumNumberOfFrames(4);
    mhdStreamer->setMainDevice(Host::getInstance());
    DynamicData::pointer image = mhdStreamer->getOutput();
    unsigned long currentTimestamp = image->getTimestamp();
    CHECK_THROWS(
        mhdStreamer->update(); // this starts the streamer
        while(!image->hasReachedEnd()) {
            mhdStreamer->update();
            if(currentTimestamp != image->getTimestamp()) {
                currentTimestamp = image->getTimestamp();
                Image::pointer frame = image->getNextFrame(PO);
            }
            // Must make this thread sleep a little so that streamer will get a chance to import data
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
    );
}
*/

TEST_CASE("ImageFileStreamer with streaming mode PROCESS_ALL and maximum number of frames stops producing when limit is reached", "[fast][ImageFileStreamer]") {
    DummyProcessObject::pointer PO = DummyProcessObject::New();
    ImageFileStreamer::pointer mhdStreamer = ImageFileStreamer::New();
    mhdStreamer->setFilenameFormat(Config::getTestDataPath()+"US/Ball/US-3Dt_#.mhd");
    mhdStreamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
    mhdStreamer->setMaximumNumberOfFrames(4);
    mhdStreamer->setMainDevice(Host::getInstance());
    mhdStreamer->update(); // this starts the streamer
    DynamicData::pointer image = mhdStreamer->getOutputData<Image>(0);
    unsigned long currentTimestamp = image->getTimestamp();
    // Must make this thread sleep a little so that streamer will get a chance to import data
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    CHECK(image->getSize() == 4);
}

TEST_CASE("ImageFileStreamer with streaming mode STORE_ALL and maximum number of frames set", "[fast][ImageFileStreamer]") {
    DummyProcessObject::pointer PO = DummyProcessObject::New();
    ImageFileStreamer::pointer mhdStreamer = ImageFileStreamer::New();
    mhdStreamer->setFilenameFormat(Config::getTestDataPath()+"US/Ball/US-3Dt_#.mhd");
    mhdStreamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
    mhdStreamer->setMaximumNumberOfFrames(4);
    mhdStreamer->setMainDevice(Host::getInstance());
    CHECK_NOTHROW(
        mhdStreamer->update(); // this starts the streamer
        DynamicData::pointer image = mhdStreamer->getOutputData<Image>(0);
        unsigned long currentTimestamp = image->getTimestamp();
        while(!image->hasReachedEnd()) {
            mhdStreamer->update();
            if(currentTimestamp != image->getTimestamp()) {
                currentTimestamp = image->getTimestamp();
                Image::pointer frame = image->getNextFrame(PO);
            }
            // Must make this thread sleep a little so that streamer will get a chance to import data
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
    );
}

TEST_CASE("ImageFileStreamer streaming to OpenCL device with streaming mode NEWEST set", "[fast][ImageFileStreamer]") {
    DummyProcessObject::pointer PO = DummyProcessObject::New();
    DeviceManager* deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager->getOneOpenCLDevice();
    ImageFileStreamer::pointer mhdStreamer = ImageFileStreamer::New();
    mhdStreamer->setStreamingMode(STREAMING_MODE_NEWEST_FRAME_ONLY);
    mhdStreamer->setFilenameFormat(Config::getTestDataPath()+"US/Ball/US-3Dt_#.mhd");
    mhdStreamer->setMainDevice(device);
    CHECK_NOTHROW(
    mhdStreamer->update(); // this starts the streamer
    DynamicData::pointer image = mhdStreamer->getOutputData<Image>(0);
    unsigned long currentTimestamp = image->getTimestamp();
    while(!image->hasReachedEnd()) {
        mhdStreamer->update();
        if(currentTimestamp != image->getTimestamp()) {
            currentTimestamp = image->getTimestamp();
            Image::pointer frame = image->getNextFrame(PO);
        }
        // Must make this thread sleep a little so that streamer will get a chance to import data
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    );
}

TEST_CASE("ImageFileStreamer streaming to OpenCL device with streaming mode PROCESS_ALL set", "[fast][ImageFileStreamer]") {
    DummyProcessObject::pointer PO = DummyProcessObject::New();
    DeviceManager* deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager->getOneOpenCLDevice();
    ImageFileStreamer::pointer mhdStreamer = ImageFileStreamer::New();
    mhdStreamer->setFilenameFormat(Config::getTestDataPath()+"US/Ball/US-3Dt_#.mhd");
    mhdStreamer->setMainDevice(device);
    mhdStreamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
    CHECK_NOTHROW(
    mhdStreamer->update(); // this starts the streamer
    DynamicData::pointer image = mhdStreamer->getOutputData<Image>(0);
    unsigned long currentTimestamp = image->getTimestamp();
    while(!image->hasReachedEnd()) {
        mhdStreamer->update();
        if(currentTimestamp != image->getTimestamp()) {
            currentTimestamp = image->getTimestamp();
            Image::pointer frame = image->getNextFrame(PO);
        }
        // Must make this thread sleep a little so that streamer will get a chance to import data
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    );
}

TEST_CASE("ImageFileStreamer streaming to OpenCL device with streaming mode STORE_ALL set", "[fast][ImageFileStreamer]") {
    DummyProcessObject::pointer PO = DummyProcessObject::New();
    DeviceManager* deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager->getOneOpenCLDevice();
    ImageFileStreamer::pointer mhdStreamer = ImageFileStreamer::New();
    mhdStreamer->setFilenameFormat(Config::getTestDataPath()+"US/Ball/US-3Dt_#.mhd");
    mhdStreamer->setMainDevice(device);
    mhdStreamer->setStreamingMode(STREAMING_MODE_STORE_ALL_FRAMES);
    CHECK_NOTHROW(
    mhdStreamer->update(); // this starts the streamer
    DynamicData::pointer image = mhdStreamer->getOutputData<Image>(0);
    unsigned long currentTimestamp = image->getTimestamp();
    while(!image->hasReachedEnd()) {
        mhdStreamer->update();
        if(currentTimestamp != image->getTimestamp()) {
            currentTimestamp = image->getTimestamp();
            Image::pointer frame = image->getNextFrame(PO);
        }
        // Must make this thread sleep a little so that streamer will get a chance to import data
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    );
}
