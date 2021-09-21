#pragma once

#include "FAST/Streamers/Streamer.hpp"
#include "FAST/ProcessObject.hpp"
#include <thread>

namespace fast {

class FAST_EXPORT  TransformFileStreamer : public Streamer {
    FAST_PROCESS_OBJECT(TransformFileStreamer)
    public:
        /**
         * @brief Create instance
         * @param filename File to stream from
         * @param timestampFilename Timestamp file
         * @return instance
         */
        FAST_CONSTRUCTOR(TransformFileStreamer,
                         std::string, filename,,
                         std::string, timestampFilename,
        );
        void setFilename(std::string str);
        void setTimestampFilename(std::string filepath);
        void enableLooping();
        void disableLooping();
        /**
         * Set a sleep time after each frame is read
         */
        void setSleepTime(uint milliseconds);
        uint getNrOfFrames() const;
        /**
         * This method runs in a separate thread and adds frames to the
         * output object
         */
        void generateStream() override;

        ~TransformFileStreamer();
    private:
        TransformFileStreamer();

        // Update the streamer if any parameters have changed
        void execute();

        bool mLoop;
        uint mSleepTime;

        uint mNrOfFrames;
        uint mMaximumNrOfFrames;
        bool mMaximumNrOfFramesSet;

        std::string mFilename;
        std::string mTimestampFilename;

};

} // end namespace fast
