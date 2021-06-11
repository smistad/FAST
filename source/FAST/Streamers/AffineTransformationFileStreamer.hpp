#pragma once

#include "FAST/Streamers/Streamer.hpp"
#include "FAST/ProcessObject.hpp"
#include <thread>

namespace fast {

class FAST_EXPORT  AffineTransformationFileStreamer : public Streamer {
    FAST_PROCESS_OBJECT(AffineTransformationFileStreamer)
    public:
        FAST_CONSTRUCTOR(AffineTransformationFileStreamer,
                         std::string, filename,,
                         std::string, timestampFilename,
        )
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

        ~AffineTransformationFileStreamer();
    private:
        AffineTransformationFileStreamer();

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
