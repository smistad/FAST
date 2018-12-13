#include "KinectStreamer.hpp"
#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/frame_listener_impl.h>
#include <libfreenect2/registration.h>
#include "FAST/Data/Image.hpp"
#include "FAST/Data/Mesh.hpp"

namespace fast {


bool KinectStreamer::mInitialized = false;
std::stack<std::string> KinectStreamer::mAvailableDevices;

KinectStreamer::KinectStreamer() {
    createOutputPort<Image>(0); // RGB
    createOutputPort<Image>(1); // Depth image
    createOutputPort<Mesh>(2); // Point cloud
    mNrOfFrames = 0;
    mHasReachedEnd = false;
    mFirstFrameIsInserted = false;
    mStreamIsStarted = false;
    mIsModified = true;
    mPointCloudFilterEnabled = false;
    registration = NULL;
}

void KinectStreamer::setPointCloudFiltering(bool enabled) {
    mPointCloudFilterEnabled = enabled;
}

void KinectStreamer::execute() {
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


MeshVertex KinectStreamer::getPoint(int x, int y) {
    float x2, y2, z2, rgb;
    Color color;
    if(registration != NULL) {
        registration->getPointXYZRGB(mUndistorted, mRegistered, y, 512-x-1, x2, y2, z2, rgb);
        const uint8_t *p = reinterpret_cast<uint8_t*>(&rgb);
        uint8_t red = p[0];
        uint8_t green = p[1];
        uint8_t blue = p[2];
        color = Color(red/255.0f, green/255.0f, blue/255.0f);
    } else {
        throw Exception();
    }

    MeshVertex vertex(Vector3f(-x2*1000, y2*1000, z2*1000));
    vertex.setColor(color);
    return vertex;
}

void KinectStreamer::producerStream() {
    reportInfo() << "Trying to set up kinect stream..." << reportEnd();
    libfreenect2::Freenect2 freenect2;
    libfreenect2::Freenect2Device *dev = 0;
    libfreenect2::PacketPipeline *pipeline = 0;
    std::string serial = "";

    int nrOfDevices = freenect2.enumerateDevices();
    if(nrOfDevices == 0) {
        throw Exception("Unable to find any Kinect devices");
    }
    if(!mInitialized) {
        for(int i = 0; i < nrOfDevices; ++i) {
            mAvailableDevices.push(freenect2.getDeviceSerialNumber(i));
        }
        mInitialized = true;
    }
    if(mAvailableDevices.empty()) {
        throw Exception("No more available kinect devices for KinectStreamer");
    } else {
        // Select first
        serial = mAvailableDevices.top();
        mAvailableDevices.pop();
    }

    reportInfo() << "Using kinect device with serial: " << serial << reportEnd();
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

    registration = new libfreenect2::Registration(dev->getIrCameraParams(),
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
        mUndistorted = &undistorted;
        mRegistered = &registered;

        auto * depth_data = (float*)undistorted.data;
        auto * rgb_data = (unsigned char*)registered.data;

        Image::pointer depthImage = Image::New();
        depthImage->create(512, 424, TYPE_FLOAT, 1, depth_data);

        if(rgb->format == libfreenect2::Frame::Format::BGRX) {
            // Have to swap B and R channel
            for(int i = 0; i < 512*424; ++i) {
                uchar blue = rgb_data[i*4];
                rgb_data[i*4] = rgb_data[i*4+2];
                rgb_data[i*4+2] = blue;
            }
        }

        auto rgbImage = Image::New();
        rgbImage->create(512, 424, TYPE_UINT8, 4, rgb_data);
        auto imageAccess = rgbImage->getImageAccess(ACCESS_READ_WRITE);
        uchar* rgb_data2 = (uchar*)imageAccess->get();
        auto depthAccess = depthImage->getImageAccess(ACCESS_READ_WRITE);
        float* depth_data2 = (float*)depthAccess->get();

        // Create point cloud
        std::vector<MeshVertex> points;
        for(int r=0; r<424; ++r) { // y
            for(int c = 0; c < 512; ++c) { // x
                // Flip image horizontally
                if(c < 512/2) {
                    for(uchar i = 0; i < 3; i++) {
                        std::swap(rgb_data2[(c + r * 512) * 4 + i], rgb_data2[(512 - c - 1 + r * 512) * 4 + i]);
                    }
                    std::swap(depth_data2[(c + r * 512)], depth_data2[(512 - c - 1 + r * 512)]);
                }
                float x, y, z, color;
                registration->getPointXYZRGB(&undistorted, &registered, r, c, x, y, z, color);
                if(z < mMinRange || z > mMaxRange) {
                    continue;
                }
                if(!std::isnan(x)) {
                    // for each valid neigbhor; is the depth similar
                    if(mPointCloudFilterEnabled) {
                        int invalidNeighbors = 0;
                        const int size = 1;
                        for(int a = -size; a <= size; ++a) {
                            for(int b = -size; b <= size; ++b) {
                                if(r + a < 0 || r + a >= 424 || c + b < 0 || c + b >= 512)
                                    continue;
                                float x2, y2, z2, color2;
                                registration->getPointXYZRGB(&undistorted, &registered, r + a, c + b, x2, y2, z2,
                                                             color2);
                                if(std::isnan(x2))
                                    invalidNeighbors++;
                                if(fabs(z - z2) * 1000 > 10)
                                    invalidNeighbors++;

                            }
                        }
                        if (invalidNeighbors > 0)
                            continue;
                    }

                    // Decode color channels
                    const uint8_t *p = reinterpret_cast<uint8_t*>(&color);
                    uint8_t red = p[0];
                    uint8_t green = p[1];
                    uint8_t blue = p[2];
                    MeshVertex point(Vector3f(-x*1000, y*1000, z*1000)); // Flip x
                    point.setColor(Color(red/255.0f, green/255.0f, blue/255.0f));
                    points.push_back(point);
                }
            }
        }
        auto cloud = Mesh::New();
        cloud->create(points);
        imageAccess->release();
        depthAccess->release();

        addOutputData(0, rgbImage);
        addOutputData(1, depthImage);
        addOutputData(2, cloud);
        if(!mFirstFrameIsInserted) {
            {
                std::lock_guard<std::mutex> lock(mFirstFrameMutex);
                mFirstFrameIsInserted = true;
            }
            mFirstFrameCondition.notify_one();
        }
        mNrOfFrames++;
        listener.release(frames);
    }

    dev->stop();
    dev->close();
    delete dev;
    reportInfo() << "Kinect streamer stopped" << Reporter::end();
    mAvailableDevices.push(serial); // Adding streamer back to queue
}

bool KinectStreamer::hasReachedEnd() {
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
    reportInfo() << "Stopping kinect streamer" << Reporter::end();
    mStop = true;
}

void KinectStreamer::setMaxRange(float range) {
    if(range < 0)
        throw Exception("Range has to be >= 0");
    mMaxRange = range;
}

void KinectStreamer::setMinRange(float range) {
    if(range < 0)
        throw Exception("Range has to be >= 0");
    mMinRange = range;
}

}