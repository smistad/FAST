#include "KinectStreamer.hpp"
#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/frame_listener_impl.h>
#include <libfreenect2/registration.h>
#include "FAST/Data/Image.hpp"
#include "FAST/Data/PointSet.hpp"

namespace fast {

KinectStreamer::KinectStreamer() {
    createOutputPort<Image>(0, OUTPUT_DYNAMIC); // RGB
    createOutputPort<Image>(1, OUTPUT_DYNAMIC); // Depth image
    createOutputPort<PointSet>(2, OUTPUT_DYNAMIC); // Point cloud
    mNrOfFrames = 0;
    mHasReachedEnd = false;
    mFirstFrameIsInserted = false;
    mIsModified = true;
}

void KinectStreamer::execute() {
    getOutputData<Image>(0)->setStreamer(mPtr.lock());
    getOutputData<Image>(1)->setStreamer(mPtr.lock());
    getOutputData<Image>(2)->setStreamer(mPtr.lock());
    if(!mStreamIsStarted) {
        // Check that first frame exists before starting streamer

        mStreamIsStarted = true;
        mStop = false;
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

    if(freenect2.enumerateDevices() == 0) {
        throw Exception("Unable to find any Kinect devices");
    }
    if(serial == "") {
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

    reportInfo() << "Kinect device serial: " << dev->getSerialNumber() << reportEnd();
    reportInfo() << "Kinect device firmware: " << dev->getFirmwareVersion() << reportEnd();

    libfreenect2::Registration *registration = new libfreenect2::Registration(dev->getIrCameraParams(),
                                                                              dev->getColorCameraParams());
    libfreenect2::Frame undistorted(512, 424, 4), registered(512, 424, 4);

    while(true) {
        {
            // Check if stop signal is sent
            std::unique_lock<std::mutex> lock(mStopMutex);
            if(mStop) {
                mStreamIsStarted = false;
                mFirstFrameIsInserted = false;
                mHasReachedEnd = false;
                break;
            }
        }
        if(!listener.waitForNewFrame(frames, 10 * 1000)) { // 10 seconds
            throw Exception("Kinect streaming timeout");
        }
        libfreenect2::Frame *rgb = frames[libfreenect2::Frame::Color];
        libfreenect2::Frame *ir = frames[libfreenect2::Frame::Ir];
        libfreenect2::Frame *depth = frames[libfreenect2::Frame::Depth];

        registration->apply(rgb, depth, &undistorted, &registered);

        float* depth_data = (float*)undistorted.data;
        unsigned char* rgb_data = (unsigned char*)registered.data;

        Image::pointer depthImage = Image::New();
        depthImage->create(512, 424, TYPE_FLOAT, 1, depth_data);

        Image::pointer rgbImage = Image::New();
        if(rgb->format == libfreenect2::Frame::Format::BGRX) {
            // Have to swap B and R channel
            for(int i = 0; i < 512*424; ++i) {
                uchar blue = rgb_data[i*4];
                rgb_data[i*4] = rgb_data[i*4+2];
                rgb_data[i*4+2] = blue;
            }
        }
        rgbImage->create(512, 424, TYPE_UINT8, 4, rgb_data);

        // Create point cloud
        std::vector<Vector3f> points;
        for(int r=0; r<424; ++r) {
            for(int c = 0; c < 512; ++c) {
                float x, y, z, color;
                registration->getPointXYZRGB(&undistorted, &registered, r, c, x, y, z, color);
                if(!std::isnan(x)) {
                    Vector3f point(x*1000, y*1000, z*1000);
                    //std::cout << point.transpose() << std::endl;
                    points.push_back(point);
                }
            }
        }
        PointSet::pointer cloud = PointSet::New();
        cloud->create(points);

        DynamicData::pointer ddRGB = getOutputData<Image>(0);
        DynamicData::pointer ddDepth = getOutputData<Image>(1);
        DynamicData::pointer ddPoint = getOutputData<PointSet>(2);
        if(ddRGB.isValid() && ddDepth.isValid()) {
            try {
                ddRGB->addFrame(rgbImage);
                ddDepth->addFrame(depthImage);
                ddPoint->addFrame(cloud);
            } catch(NoMoreFramesException &e) {
                throw e;
            } catch(Exception &e) {
                reportInfo() << "streamer has been deleted, stop" << Reporter::end;
                listener.release(frames);
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
            listener.release(frames);
            break;
        }
        mNrOfFrames++;
        listener.release(frames);
    }

    reportInfo() << "Stopping kinect streamer" << Reporter::end;
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

KinectStreamer::~KinectStreamer() {
    if(mStreamIsStarted) {
        stop();
        mThread->join();
    }
}

void KinectStreamer::stop() {
    std::unique_lock<std::mutex> lock(mStopMutex);
    mStop = true;
}

}