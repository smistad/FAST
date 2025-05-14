#pragma once

#include <FAST/Streamers/RandomAccessStreamer.hpp>

namespace fast {

class Image;

/**
 * @brief Stream and provide random access to frames from video files
 *
 * This streamer loads all frames into memory at start using VideoStreamer
 * to provide random access to frames. If random access is not needed,
 * consider using VideoStreamer instead.
 *
 * @ingroup streamers
 * @sa VideoStreamer
 */
class FAST_EXPORT VideoRandomAccessStreamer : public RandomAccessStreamer {
    FAST_PROCESS_OBJECT(VideoRandomAccessStreamer)
    public:
    /**
       * @brief Create instance
       * @param filename Movie file to stream from
       * @param loop Whether to loop the video or not
       * @param useFramerate Use framerate in video file.
       *    If false, and framerate is -1, this streamer will stream as fast as possible.
       * @param framerate Specify custom framerate to use.
       * @param grayscale Whether to convert to grayscale or not
       * @return instance
       */
        FAST_CONSTRUCTOR(VideoRandomAccessStreamer,
                         std::string, filename,,
                         bool, loop, = false,
                         bool, useFramerate, = true,
                         int, framerate, = -1,
                         bool, grayscale, = true
         );
        /**
         * @brief Load all frames from video
         */
        void load();
        /**
         * @brief Get nr of frames
         * This will load all frames from the video file to memory.
         * @return
         */
        int getNrOfFrames() override;
        void setFilename(std::string filename);
        std::string getFilename() const;
        void setGrayscale(bool grayscale);
        bool getGrayscale() const;
    private:
        VideoRandomAccessStreamer();
        void execute() override;
        void generateStream() override;

        std::vector<std::shared_ptr<Image>> m_frames;
        std::string m_filename;
        bool m_grayscale = false;
        bool m_useFramerate = true;
};

}