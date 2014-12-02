#include "catch.hpp"
#include "DeviceManager.hpp"
#include <boost/thread.hpp>
#include "ImageFileStreamer.hpp"
#include "DummyObjects.hpp"

using namespace fast;

TEST_CASE("No filename format given to ImageFileStreamer", "[fast][ImageFileStreamer]") {
    ImageFileStreamer::pointer mhdStreamer = ImageFileStreamer::New();
    DynamicImage::pointer image = mhdStreamer->getOutput();
    CHECK_THROWS(image->update());
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
    DynamicImage::pointer image = mhdStreamer->getOutput();
    std::cout << "asd" << std::endl;
    CHECK_THROWS(image->update());
}
*/

TEST_CASE("ImageFileStreamer streaming to host with streaming mode NEWEST set", "[fast][ImageFileStreamer]") {
    DummyProcessObject::pointer PO = DummyProcessObject::New();
    ImageFileStreamer::pointer mhdStreamer = ImageFileStreamer::New();
    mhdStreamer->setFilenameFormat(std::string(FAST_TEST_DATA_DIR)+"US-3Dt/US-3Dt_#.mhd");
    mhdStreamer->setStreamingMode(STREAMING_MODE_NEWEST_FRAME_ONLY);
    mhdStreamer->setDevice(Host::New());
    DynamicImage::pointer image = mhdStreamer->getOutput();
    unsigned long currentTimestamp = image->getTimestamp();
    CHECK_NOTHROW(
    image->update(); // this starts the streamer
    while(!image->hasReachedEnd()) {
        image->update();
        if(currentTimestamp != image->getTimestamp()) {
            currentTimestamp = image->getTimestamp();
            Image::pointer frame = image->getNextFrame(PO);
        }
        // Must make this thread sleep a little so that streamer will get a chance to import data
        boost::this_thread::sleep(boost::posix_time::milliseconds(20));
    }
    );
}

TEST_CASE("ImageFileStreamer streaming to host with streaming mode PROCESS_ALL set", "[fast][ImageFileStreamer]") {
    DummyProcessObject::pointer PO = DummyProcessObject::New();
    ImageFileStreamer::pointer mhdStreamer = ImageFileStreamer::New();
    mhdStreamer->setFilenameFormat(std::string(FAST_TEST_DATA_DIR)+"US-3Dt/US-3Dt_#.mhd");
    mhdStreamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
    mhdStreamer->setDevice(Host::New());
    DynamicImage::pointer image = mhdStreamer->getOutput();
    unsigned long currentTimestamp = image->getTimestamp();
    CHECK_NOTHROW(
    image->update(); // this starts the streamer
    while(!image->hasReachedEnd()) {
        image->update();
        if(currentTimestamp != image->getTimestamp()) {
            currentTimestamp = image->getTimestamp();
            Image::pointer frame = image->getNextFrame(PO);
        }
        // Must make this thread sleep a little so that streamer will get a chance to import data
        boost::this_thread::sleep(boost::posix_time::milliseconds(20));
    }
    );
}

TEST_CASE("ImageFileStreamer streaming to host with streaming mode STORE_ALL set", "[fast][ImageFileStreamer]") {
    DummyProcessObject::pointer PO = DummyProcessObject::New();
    ImageFileStreamer::pointer mhdStreamer = ImageFileStreamer::New();
    mhdStreamer->setFilenameFormat(std::string(FAST_TEST_DATA_DIR)+"US-3Dt/US-3Dt_#.mhd");
    mhdStreamer->setStreamingMode(STREAMING_MODE_STORE_ALL_FRAMES);
    mhdStreamer->setDevice(Host::New());
    DynamicImage::pointer image = mhdStreamer->getOutput();
    unsigned long currentTimestamp = image->getTimestamp();
    CHECK_NOTHROW(
    image->update(); // this starts the streamer
    while(!image->hasReachedEnd()) {
        image->update();
        if(currentTimestamp != image->getTimestamp()) {
            currentTimestamp = image->getTimestamp();
            Image::pointer frame = image->getNextFrame(PO);
        }
        // Must make this thread sleep a little so that streamer will get a chance to import data
        boost::this_thread::sleep(boost::posix_time::milliseconds(20));
    }
    );
}

TEST_CASE("ImageFileStreamer streaming to OpenCL device with streaming mode NEWEST set", "[fast][ImageFileStreamer]") {
    DummyProcessObject::pointer PO = DummyProcessObject::New();
    DeviceManager& deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager.getOneOpenCLDevice();
    ImageFileStreamer::pointer mhdStreamer = ImageFileStreamer::New();
    mhdStreamer->setStreamingMode(STREAMING_MODE_NEWEST_FRAME_ONLY);
    mhdStreamer->setFilenameFormat(std::string(FAST_TEST_DATA_DIR)+"US-3Dt/US-3Dt_#.mhd");
    mhdStreamer->setDevice(device);
    DynamicImage::pointer image = mhdStreamer->getOutput();
    unsigned long currentTimestamp = image->getTimestamp();
    CHECK_NOTHROW(
    image->update(); // this starts the streamer
    while(!image->hasReachedEnd()) {
        image->update();
        if(currentTimestamp != image->getTimestamp()) {
            currentTimestamp = image->getTimestamp();
            Image::pointer frame = image->getNextFrame(PO);
        }
        // Must make this thread sleep a little so that streamer will get a chance to import data
        boost::this_thread::sleep(boost::posix_time::milliseconds(20));
    }
    );
}

TEST_CASE("ImageFileStreamer streaming to OpenCL device with streaming mode PROCESS_ALL set", "[fast][ImageFileStreamer]") {
    DummyProcessObject::pointer PO = DummyProcessObject::New();
    DeviceManager& deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager.getOneOpenCLDevice();
    ImageFileStreamer::pointer mhdStreamer = ImageFileStreamer::New();
    mhdStreamer->setFilenameFormat(std::string(FAST_TEST_DATA_DIR)+"US-3Dt/US-3Dt_#.mhd");
    mhdStreamer->setDevice(device);
    mhdStreamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
    DynamicImage::pointer image = mhdStreamer->getOutput();
    unsigned long currentTimestamp = image->getTimestamp();
    CHECK_NOTHROW(
    image->update(); // this starts the streamer
    while(!image->hasReachedEnd()) {
        image->update();
        if(currentTimestamp != image->getTimestamp()) {
            currentTimestamp = image->getTimestamp();
            Image::pointer frame = image->getNextFrame(PO);
        }
        // Must make this thread sleep a little so that streamer will get a chance to import data
        boost::this_thread::sleep(boost::posix_time::milliseconds(20));
    }
    );
}

TEST_CASE("ImageFileStreamer streaming to OpenCL device with streaming mode STORE_ALL set", "[fast][ImageFileStreamer]") {
    DummyProcessObject::pointer PO = DummyProcessObject::New();
    DeviceManager& deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager.getOneOpenCLDevice();
    ImageFileStreamer::pointer mhdStreamer = ImageFileStreamer::New();
    mhdStreamer->setFilenameFormat(std::string(FAST_TEST_DATA_DIR)+"US-3Dt/US-3Dt_#.mhd");
    mhdStreamer->setDevice(device);
    mhdStreamer->setStreamingMode(STREAMING_MODE_STORE_ALL_FRAMES);
    DynamicImage::pointer image = mhdStreamer->getOutput();
    unsigned long currentTimestamp = image->getTimestamp();
    CHECK_NOTHROW(
    image->update(); // this starts the streamer
    while(!image->hasReachedEnd()) {
        image->update();
        if(currentTimestamp != image->getTimestamp()) {
            currentTimestamp = image->getTimestamp();
            Image::pointer frame = image->getNextFrame(PO);
        }
        // Must make this thread sleep a little so that streamer will get a chance to import data
        boost::this_thread::sleep(boost::posix_time::milliseconds(20));
    }
    );
}
