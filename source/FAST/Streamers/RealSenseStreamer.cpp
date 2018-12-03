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

}

static bool profile_changed(const std::vector<rs2::stream_profile>& current, const std::vector<rs2::stream_profile>& prev)
{
    for (auto&& sp : prev)
    {
        //If previous profile is in current (maybe just added another)
        auto itr = std::find_if(std::begin(current), std::end(current), [&sp](const rs2::stream_profile& current_sp) { return sp.unique_id() == current_sp.unique_id(); });
        if (itr == std::end(current)) //If it previous stream wasn't found in current
        {
            return true;
        }
    }
    return false;
}

static rs2_stream find_stream_to_align(const std::vector<rs2::stream_profile>& streams)
{
    //Given a vector of streams, we try to find a depth stream and another stream to align depth with.
    //We prioritize color streams to make the view look better.
    //If color is not available, we take another stream that (other than depth)
    rs2_stream align_to = RS2_STREAM_ANY;
    bool depth_stream_found = false;
    bool color_stream_found = false;
    for (rs2::stream_profile sp : streams)
    {
        rs2_stream profile_stream = sp.stream_type();
        if (profile_stream != RS2_STREAM_DEPTH)
        {
            if (!color_stream_found)         //Prefer color
                align_to = profile_stream;

            if (profile_stream == RS2_STREAM_COLOR)
            {
                color_stream_found = true;
            }
        }
        else
        {
            depth_stream_found = true;
        }
    }

    if(!depth_stream_found)
        throw std::runtime_error("No Depth stream available");

    if (align_to == RS2_STREAM_ANY)
        throw std::runtime_error("No stream found to align with Depth");

    return align_to;
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
    throw std::runtime_error("Device does not have a depth sensor");
}

/**
Class to encapsulate a filter alongside its options
*/
class filter_options
{
    public:
        filter_options(const std::string name, rs2::filter& filter);
        filter_options(filter_options&& other);
        std::string filter_name;                                   //Friendly name of the filter
        rs2::filter& filter;                                       //The filter in use
        std::atomic_bool is_enabled;                               //A boolean controlled by the user that determines whether to apply the filter or not
};

/**
Constructor for filter_options, takes a name and a filter.
*/
filter_options::filter_options(const std::string name, rs2::filter& flt) :
        filter_name(name),
        filter(flt),
        is_enabled(true)
{
}

filter_options::filter_options(filter_options&& other) :
        filter_name(std::move(other.filter_name)),
        filter(other.filter),
        is_enabled(other.is_enabled.load())
{
}

void RealSenseStreamer::producerStream() {
    reportInfo() << "Trying to set up real sense stream..." << reportEnd();

    // Create a Pipeline - this serves as a top-level API for streaming and processing frames
    rs2::pipeline pipeline;

    // Configure and start the pipeline
    rs2::pipeline_profile profile;
    try {
        profile = pipeline.start();
    } catch(rs2::error &e) {
        throw Exception("Error could not start real sense streaming pipeline: " + std::string(e.what()));
    }

    const float depth_scale = get_depth_scale(profile.get_device());

    //Pipeline could choose a device that does not have a color stream
    //If there is no color stream, choose to align depth to another stream
    rs2_stream align_to = find_stream_to_align(profile.get_streams());
    auto stream = profile.get_stream(RS2_STREAM_DEPTH).as<rs2::video_stream_profile>();
    auto intrinsics = stream.get_intrinsics(); // Calibration data

    // Create a rs2::align object.
    // rs2::align allows us to perform alignment of depth frames to others frames
    //The "align_to" is the stream type to which we plan to align depth frames.
    rs2::align align(align_to);

    // Declare filters
    rs2::decimation_filter dec_filter;  // Decimation - reduces depth frame density
    rs2::spatial_filter spat_filter;    // Spatial    - edge-preserving spatial smoothing
    rs2::temporal_filter temp_filter;   // Temporal   - reduces temporal noise

    // Declare disparity transform from depth to disparity and vice versa
    const std::string disparity_filter_name = "Disparity";
    rs2::disparity_transform depth_to_disparity(true);
    rs2::disparity_transform disparity_to_depth(false);


    // Initialize a vector that holds filters and their options
    std::vector<filter_options> filters;

    // The following order of emplacment will dictate the orders in which filters are applied
    //filters.emplace_back("Decimate", dec_filter);
    filters.emplace_back(disparity_filter_name, depth_to_disparity);
    filters.emplace_back("Spatial", spat_filter);
    filters.emplace_back("Temporal", temp_filter);

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
            auto processed = align.process(frames);

            // Trying to get both other and aligned depth frames
            rs2::video_frame color_frame = processed.first(align_to);
            rs2::depth_frame depth_frame = processed.get_depth_frame(); // Aligned depth frame

            //If one of them is unavailable, continue iteration
            if(!depth_frame || !color_frame)
                continue;

            rs2::frame filtered = depth_frame; // Does not copy the frame, only adds a reference

            /* Apply filters.
            The implemented flow of the filters pipeline is in the following order:
            1. apply decimation filter
            2. transform the scence into disparity domain
            3. apply spatial filter
            4. apply temporal filter
            5. revert the results back (if step Disparity filter was applied
            to depth domain (each post processing block is optional and can be applied independantly).
            */
            bool revert_disparity = false;
            for (auto&& filter : filters)
            {
                if (filter.is_enabled)
                {
                    filtered = filter.filter.process(filtered);
                    if (filter.filter_name == disparity_filter_name)
                    {
                        revert_disparity = true;
                    }
                }
            }
            if (revert_disparity)
            {
                filtered = disparity_to_depth.process(filtered);
            }

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
        int width = static_cast<rs2::video_frame>(depth_frame).get_width();
        int height = static_cast<rs2::video_frame>(depth_frame).get_height();
        //std::cout << "depth size: " << width << " " << height << std::endl;

        const uint16_t* p_depth_frame = reinterpret_cast<const uint16_t*>(depth_frame.get_data());
        uint8_t* p_other_frame = reinterpret_cast<uint8_t*>(const_cast<void*>(color_frame.get_data()));

        width = static_cast<rs2::video_frame>(color_frame).get_width();
        height = static_cast<rs2::video_frame>(color_frame).get_height();
        //std::cout << "color size: " << width << " " << height << std::endl;
        int other_bpp = static_cast<rs2::video_frame>(color_frame).get_bytes_per_pixel();

        std::unique_ptr<float[]> depthData = std::make_unique<float[]>(width*height);

        std::vector<MeshVertex> points;
        for(int y = 0; y < height; y++) {
            auto depth_pixel_index = y * width;
            for(int x = 0; x < width; x++, ++depth_pixel_index) {
                // Get the depth value of the current pixel
                float pixels_distance = depth_scale * p_depth_frame[depth_pixel_index];
                depthData[depth_pixel_index] = pixels_distance * 1000; // To mm

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
                    rs2_deproject_pixel_to_point(upoint, &intrinsics, upixel, pixels_distance);
                    MeshVertex vertex(Vector3f(upoint[0]*1000, upoint[1]*1000, upoint[2]*1000)); // Convert to mm
                    vertex.setColor(Color(p_other_frame[offset]/255.0f, p_other_frame[offset+1]/255.0f, p_other_frame[offset+2]/255.0f));
                    points.push_back(vertex);
                }
            }
        }

        // Create depth image
        Image::pointer depthImage = Image::New();
        depthImage->create(width, height, TYPE_FLOAT, 1, std::move(depthData));

        // Create mesh
        Mesh::pointer cloud = Mesh::New();
        cloud->create(points);

        // Create RGB camera image
        std::unique_ptr<uint8_t[]> colorData = std::make_unique<uint8_t[]>(width*height*3);
        std::memcpy(colorData.get(), p_other_frame, width*height*sizeof(uint8_t)*3);
        Image::pointer colorImage = Image::New();
        colorImage->create(width, height, TYPE_UINT8, 3, std::move(colorData));

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