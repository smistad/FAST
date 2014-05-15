#include "catch.hpp"
#include "MetaImageStreamer.hpp"
#include "DeviceManager.hpp"
#include <boost/thread.hpp>

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

TEST_CASE("MetaImageStreamer streaming to host with no streaming mode set", "[fast][MetaImageStreamer]") {
    MetaImageStreamer::pointer mhdStreamer = MetaImageStreamer::New();
    mhdStreamer->setFilenameFormat(std::string(FAST_ROOT_DIR)+"TestData/US-3Dt/US-3Dt_#.mhd");
    mhdStreamer->setDevice(Host::New());
    DynamicImage::pointer image = mhdStreamer->getOutput();
    unsigned long currentTimestamp = image->getTimestamp();
    CHECK_NOTHROW(
    image->update(); // this starts the streamer
    while(!image->hasReachedEnd()) {
        image->update();
        if(currentTimestamp != image->getTimestamp()) {
            currentTimestamp = image->getTimestamp();
            Image::pointer frame = image->getNextFrame();
        }
        // Must make this thread sleep a little so that streamer will get a chance to import data
        boost::this_thread::sleep(boost::posix_time::milliseconds(10));
    }
    );
}

TEST_CASE("MetaImageStreamer streaming to host with streaming mode PROCESS_ALL set", "[fast][MetaImageStreamer]") {
    MetaImageStreamer::pointer mhdStreamer = MetaImageStreamer::New();
    mhdStreamer->setFilenameFormat(std::string(FAST_ROOT_DIR)+"TestData/US-3Dt/US-3Dt_#.mhd");
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
            Image::pointer frame = image->getNextFrame();
        }
        // Must make this thread sleep a little so that streamer will get a chance to import data
        boost::this_thread::sleep(boost::posix_time::milliseconds(10));
    }
    );
}

TEST_CASE("MetaImageStreamer streaming to host with streaming mode STORE_ALL set", "[fast][MetaImageStreamer]") {
    MetaImageStreamer::pointer mhdStreamer = MetaImageStreamer::New();
    mhdStreamer->setFilenameFormat(std::string(FAST_ROOT_DIR)+"TestData/US-3Dt/US-3Dt_#.mhd");
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
            Image::pointer frame = image->getNextFrame();
        }
        // Must make this thread sleep a little so that streamer will get a chance to import data
        boost::this_thread::sleep(boost::posix_time::milliseconds(10));
    }
    );
}

TEST_CASE("MetaImageStreamer streaming to OpenCL device with no streaming mode set", "[fast][MetaImageStreamer]") {
    DeviceManager& deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager.getOneOpenCLDevice();
    MetaImageStreamer::pointer mhdStreamer = MetaImageStreamer::New();
    mhdStreamer->setFilenameFormat(std::string(FAST_ROOT_DIR)+"TestData/US-3Dt/US-3Dt_#.mhd");
    mhdStreamer->setDevice(device);
    DynamicImage::pointer image = mhdStreamer->getOutput();
    unsigned long currentTimestamp = image->getTimestamp();
    CHECK_NOTHROW(
    image->update(); // this starts the streamer
    while(!image->hasReachedEnd()) {
        image->update();
        if(currentTimestamp != image->getTimestamp()) {
            currentTimestamp = image->getTimestamp();
            Image::pointer frame = image->getNextFrame();
        }
        // Must make this thread sleep a little so that streamer will get a chance to import data
        boost::this_thread::sleep(boost::posix_time::milliseconds(10));
    }
    );
}

TEST_CASE("MetaImageStreamer streaming to OpenCL device with streaming mode PROCESS_ALL set", "[fast][MetaImageStreamer]") {
    DeviceManager& deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager.getOneOpenCLDevice();
    MetaImageStreamer::pointer mhdStreamer = MetaImageStreamer::New();
    mhdStreamer->setFilenameFormat(std::string(FAST_ROOT_DIR)+"TestData/US-3Dt/US-3Dt_#.mhd");
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
            Image::pointer frame = image->getNextFrame();
        }
        // Must make this thread sleep a little so that streamer will get a chance to import data
        boost::this_thread::sleep(boost::posix_time::milliseconds(10));
    }
    );
}

TEST_CASE("MetaImageStreamer streaming to OpenCL device with streaming mode STORE_ALL set", "[fast][MetaImageStreamer]") {
    DeviceManager& deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager.getOneOpenCLDevice();
    MetaImageStreamer::pointer mhdStreamer = MetaImageStreamer::New();
    mhdStreamer->setFilenameFormat(std::string(FAST_ROOT_DIR)+"TestData/US-3Dt/US-3Dt_#.mhd");
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
            Image::pointer frame = image->getNextFrame();
        }
        // Must make this thread sleep a little so that streamer will get a chance to import data
        boost::this_thread::sleep(boost::posix_time::milliseconds(10));
    }
    );
}
