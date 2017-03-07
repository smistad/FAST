#include "KinectStreamer.hpp"
#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/frame_listener_impl.h>
#include <libfreenect2/registration.h>
#include "FAST/Data/Image.hpp"

namespace fast {

KinectStreamer::KinectStreamer() {
    createOutputPort<Image>(0, OUTPUT_DYNAMIC); // RGB
    //createOutputPort<Image>(1, OUTPUT_DYNAMIC); // Depth image
    mNrOfFrames = 0;
    mHasReachedEnd = false;
    mFirstFrameIsInserted = false;
    mIsModified = true;
}

void KinectStreamer::execute() {
    getOutputData<Image>(0)->setStreamer(mPtr.lock());
    //getOutputData<Image>(1)->setStreamer(mPtr.lock());
    if(!mStreamIsStarted) {
        // Check that first frame exists before starting streamer

        mStreamIsStarted = true;
        mThread = new std::thread(std::bind(&KinectStreamer::producerStream, this));
    }

    // Wait here for first frame
    std::unique_lock<std::mutex> lock(mFirstFrameMutex);
    while(!mFirstFrameIsInserted) {
        mFirstFrameCondition.wait(lock);
    }
}

void KinectStreamer::producerStream() {
    libfreenect2::Freenect2 freenect2;
    libfreenect2::Freenect2Device *dev = 0;
    libfreenect2::PacketPipeline *pipeline = 0;
    std::string serial = "";

    if (freenect2.enumerateDevices() == 0) {
        throw Exception("Unable to find any Kinect devices");
    }
    if (serial == "") {
        serial = freenect2.getDefaultDeviceSerialNumber();
    }
    pipeline = new libfreenect2::OpenGLPacketPipeline();
    dev = freenect2.openDevice(serial, pipeline);

    int types = libfreenect2::Frame::Color | libfreenect2::Frame::Ir | libfreenect2::Frame::Depth;
    libfreenect2::SyncMultiFrameListener listener(types);
    libfreenect2::FrameMap frames;
    dev->setColorFrameListener(&listener);
    dev->setIrAndDepthFrameListener(&listener);

    if(!dev->start())
        throw Exception("Failed to start Kinect device streaming");

    std::cout << "device serial: " << dev->getSerialNumber() << std::endl;
    std::cout << "device firmware: " << dev->getFirmwareVersion() << std::endl;

    libfreenect2::Registration *registration = new libfreenect2::Registration(dev->getIrCameraParams(),
                                                                              dev->getColorCameraParams());
    libfreenect2::Frame undistorted(512, 424, 4), registered(512, 424, 4);

    while (true) {
        if (!listener.waitForNewFrame(frames, 10 * 1000)) { // 10 seconds
            throw Exception("Kinect streaming timeout");
        }
        libfreenect2::Frame *rgb = frames[libfreenect2::Frame::Color];
        libfreenect2::Frame *ir = frames[libfreenect2::Frame::Ir];
        libfreenect2::Frame *depth = frames[libfreenect2::Frame::Depth];


        registration->apply(rgb, depth, &undistorted, &registered);

        std::cout << "Status: " << registered.status << std::endl;

        float* depth_data = (float*)undistorted.data;
        unsigned int* rgb_data = (unsigned int*)registered.data;

        Image::pointer image = Image::New();
        image->create(512, 424, TYPE_FLOAT, 1, depth_data);

        DynamicData::pointer ptr = getOutputData<Image>();
        if(ptr.isValid()) {
            try {
                ptr->addFrame(image);
            } catch(NoMoreFramesException &e) {
                throw e;
            } catch(Exception &e) {
                reportInfo() << "streamer has been deleted, stop" << Reporter::end;
                break;
            }
            if(!mFirstFrameIsInserted) {
                {
                    std::lock_guard<std::mutex> lock(mFirstFrameMutex);
                    mFirstFrameIsInserted = true;
                }
                mFirstFrameCondition.notify_one();
            }
        } else {
            reportInfo() << "DynamicImage object destroyed, stream can stop." << Reporter::end;
            break;
        }
        mNrOfFrames++;

        listener.release(frames);
    }

    dev->stop();
    dev->close();

    // TODO proper cleanup
}

bool KinectStreamer::hasReachedEnd() const {
    return mHasReachedEnd;
}

uint KinectStreamer::getNrOfFrames() const {
    return mNrOfFrames;
}

}