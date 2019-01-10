#include "RealSenseStreamer.hpp"
#include "FAST/Data/Image.hpp"
#include "FAST/Data/Mesh.hpp"
#include <librealsense2/rs.hpp>
#include "librealsense2/rsutil.h"

namespace fast {


RealSenseStreamer::RealSenseStreamer() {
    createOutputPort<Image>(0); // RGB
    createOutputPort<Image>(1); // Depth image
    createOutputPort<Mesh>(2); // Point cloud
    mNrOfFrames = 0;
    mHasReachedEnd = false;
    mFirstFrameIsInserted = false;
    mStreamIsStarted = false;
    mIsModified = true;
}

void RealSenseStreamer::execute() {
    if(!mStreamIsStarted) {
        // Check that first frame exists before starting streamer

        mStreamIsStarted = true;
        mStop = false;
        mThread = std::make_unique<std::thread>(std::bind(&RealSenseStreamer::producerStream, this));
    }

    // Wait here for first frame
    std::unique_lock<std::mutex> lock(mFirstFrameMutex);
    while(!mFirstFrameIsInserted) {
        mFirstFrameCondition.wait(lock);
    }
}


MeshVertex RealSenseStreamer::getPoint(int x, int y) {
    if(mDepthImage && mColorImage) {
        float upoint[3];
        float upixel[2] = {(float) x, (float) y};
        auto depthAccess = mDepthImage->getImageAccess(ACCESS_READ);
        float depth = depthAccess->getScalar(Vector2i(x, y)) / 1000.0f; // Get depth of current frame in meters
        rs2_deproject_pixel_to_point(upoint, intrinsics, upixel, depth);
        MeshVertex vertex(Vector3f(upoint[0] * 1000, upoint[1] * 1000, upoint[2] * 1000)); // Convert to mm
        auto colorAccess = mColorImage->getImageAccess(ACCESS_READ);
        Vector4f color = colorAccess->getVector(Vector2i(x, y));
        vertex.setColor(Color(color.x()/255.0f, color.y()/255.0f, color.z()/255.0f));  // get color of current frame
        return vertex;
    } else {
        throw Exception("Can't call getPoint before any color or depth data has arrived.");
    }
}

static float get_depth_scale(rs2::device dev)
{
    // Go over the device's sensors
    for (rs2::sensor& sensor : dev.query_sensors())
    {
        // Check if the sensor if a depth sensor
        if (rs2::depth_sensor dpt = sensor.as<rs2::depth_sensor>())
        {
            return dpt.get_depth_scale();
        }
    }
    throw Exception("Device does not have a depth sensor");
}

void RealSenseStreamer::producerStream() {
    reportInfo() << "Trying to set up real sense stream..." << reportEnd();

    // Create a Pipeline - this serves as a top-level API for streaming and processing frames
    rs2::pipeline pipeline;

    rs2::config config;
    // Use a configuration object to request only depth from the pipeline
    config.enable_stream(RS2_STREAM_DEPTH, 1280, 720, RS2_FORMAT_Z16, 30);
    config.enable_stream(RS2_STREAM_COLOR, 640, 480, RS2_FORMAT_RGB8, 30);

    // Configure and start the pipeline
    rs2::pipeline_profile profile;
    try {
        profile = pipeline.start(config);
    } catch(rs2::error &e) {
        throw Exception("Error could not start real sense streaming pipeline: " + std::string(e.what()));
    }

    const float depth_scale = get_depth_scale(profile.get_device());

    auto stream = profile.get_stream(RS2_STREAM_DEPTH).as<rs2::video_stream_profile>();
    auto tmp = stream.get_intrinsics();
    intrinsics = &tmp;

    // Create a rs2::align object.
    // rs2::align allows us to perform alignment of depth frames to others frames
    //The "align_to" is the stream type to which we plan to align depth frames.
    rs2::align align(RS2_STREAM_COLOR);

    // Declare filters
    rs2::decimation_filter dec_filter;  // Decimation - reduces depth frame density
    rs2::spatial_filter spat_filter;    // Spatial    - edge-preserving spatial smoothing
    rs2::temporal_filter temp_filter;   // Temporal   - reduces temporal noise

    // Declare disparity transform from depth to disparity and vice versa
    rs2::disparity_transform depth_to_disparity(true);
    rs2::disparity_transform disparity_to_depth(false);

    // Declaring two concurrent queues that will be used to push and pop frames from different threads
    rs2::frame_queue color_data_queue;
    rs2::frame_queue depth_data_queue;

    // Atomic boolean to allow thread safe way to stop the thread
    std::atomic_bool stopped(false);

    // Create a thread for getting frames from the device and process them
    // to prevent UI thread from blocking due to long computations.
    std::thread processing_thread([&]() {
        while (!stopped) //While application is running
        {
            rs2::frameset frames = pipeline.wait_for_frames(); // Wait for next set of frames from the camera
            //Get processed aligned frame
            rs2::frameset processed = align.process(frames);

            // Trying to get both other and aligned depth frames
            rs2::video_frame color_frame = processed.first(RS2_STREAM_COLOR);
            rs2::depth_frame depth_frame = processed.get_depth_frame(); // Aligned depth frame

            //If one of them is unavailable, continue iteration
            if(!depth_frame || !color_frame)
                continue;

            rs2::frame filtered = depth_frame; // Does not copy the frame, only adds a reference

            /* Apply filters.
            The implemented flow of the filters pipeline is in the following order:
            1. (apply decimation filter)
            2. transform the scence into disparity domain
            3. apply spatial filter
            4. apply temporal filter
            5. revert the results back (if step Disparity filter was applied
            to depth domain (each post processing block is optional and can be applied independantly).
            */
            filtered = depth_to_disparity.process(filtered);
            filtered = spat_filter.process(filtered);
            filtered = temp_filter.process(filtered);
            filtered = disparity_to_depth.process(filtered);

            // Push filtered & original data to their respective queues
            // Note, pushing to two different queues might cause the application to display
            //  original and filtered pointclouds from different depth frames
            //  To make sure they are synchronized you need to push them together or add some
            //  synchronization mechanisms
            depth_data_queue.enqueue(filtered);
            color_data_queue.enqueue(color_frame);
        }
    });


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

        // Block program until frames arrive
        rs2::frame depth_frame;
        depth_data_queue.poll_for_frame(&depth_frame);
        rs2::frame color_frame;
        color_data_queue.poll_for_frame(&color_frame);
        if(!color_frame || !depth_frame)
            continue;

        // Get the depth frame's dimensions
        const uint16_t* p_depth_frame = reinterpret_cast<const uint16_t*>(depth_frame.get_data());
        uint8_t* p_other_frame = reinterpret_cast<uint8_t*>(const_cast<void*>(color_frame.get_data()));

        int width = static_cast<rs2::video_frame>(color_frame).get_width();
        int height = static_cast<rs2::video_frame>(color_frame).get_height();
        //std::cout << "color size: " << width << " " << height << std::endl;
        int other_bpp = static_cast<rs2::video_frame>(color_frame).get_bytes_per_pixel();

        std::unique_ptr<float[]> depthData = std::make_unique<float[]>(width*height);

        std::vector<MeshVertex> points;
        for(int y = 0; y < height; y++) {
            auto depth_pixel_index = y * width;
            for(int x = 0; x < width; x++, ++depth_pixel_index) {
                // Get the depth value of the current pixel
                float pixels_distance = depth_scale * p_depth_frame[depth_pixel_index];
                depthData[depth_pixel_index] = pixels_distance * 1000.0f; // To mm

                // Calculate the offset in other frame's buffer to current pixel
                auto offset = depth_pixel_index * other_bpp;

                // Check if the depth value is invalid (<=0) or greater than the threshold
                if(pixels_distance*1000 <= mMinRange || pixels_distance*1000 > mMaxRange) {
                    // Set pixel to "background" color (0x999999)
                    //std::cout << "setting pixel to bg.." << std::endl;
                    std::memset(&p_other_frame[offset], 0x00, other_bpp);
                } else {
                    //std::cout << "creating mesh point.." << std::endl;
                    // Create point for point cloud
                    float upoint[3];
                    float upixel[2] = {(float)x, (float)y};
                    rs2_deproject_pixel_to_point(upoint, intrinsics, upixel, pixels_distance);
                    MeshVertex vertex(Vector3f(upoint[0]*1000, upoint[1]*1000, upoint[2]*1000)); // Convert to mm
                    vertex.setColor(Color(p_other_frame[offset]/255.0f, p_other_frame[offset+1]/255.0f, p_other_frame[offset+2]/255.0f));
                    points.push_back(vertex);
                }
            }
        }

        // Create depth image
        Image::pointer depthImage = Image::New();
        depthImage->create(width, height, TYPE_FLOAT, 1, std::move(depthData));
        mDepthImage = depthImage;

        // Create mesh
        Mesh::pointer cloud = Mesh::New();
        cloud->create(points);

        // Create RGB camera image
        std::unique_ptr<uint8_t[]> colorData = std::make_unique<uint8_t[]>(width*height*3);
        std::memcpy(colorData.get(), p_other_frame, width*height*sizeof(uint8_t)*3);
        Image::pointer colorImage = Image::New();
        colorImage->create(width, height, TYPE_UINT8, 3, std::move(colorData));
        mColorImage = colorImage;

        addOutputData(0, colorImage);
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
    }

    stopped = true;
    processing_thread.join();

    reportInfo() << "Real sense streamer stopped" << Reporter::end();
}

bool RealSenseStreamer::hasReachedEnd() {
    return mHasReachedEnd;
}

uint RealSenseStreamer::getNrOfFrames() const {
    return mNrOfFrames;
}

RealSenseStreamer::~RealSenseStreamer() {
    if(mStreamIsStarted) {
        stop();
        mThread->join();
    }
}

void RealSenseStreamer::stop() {
    std::unique_lock<std::mutex> lock(mStopMutex);
    reportInfo() << "Stopping real sense streamer" << Reporter::end();
    mStop = true;
}

void RealSenseStreamer::setMaxRange(float range) {
    if(range < 0)
        throw Exception("Range has to be >= 0");
    mMaxRange = range;
}

void RealSenseStreamer::setMinRange(float range) {
    if(range < 0)
        throw Exception("Range has to be >= 0");
    mMinRange = range;
}

}