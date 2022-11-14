#pragma once

#include <FAST/Streamers/Streamer.hpp>
#include <FAST/Streamers/RandomAccessStreamer.hpp>

namespace fast {

/**
 * @brief Interleaves input data streams into one output stream
 *
 * This process object interleaves multiple input data streams into one output stream
 * by alternating data objects from the input data streams.
 * This is useful for instance for comparing image quality of two or more data streams.
 * The speed in which they are interleaved can be controlled by the framerate parameter.
 *
 * Inputs:
 * - 0-N: Any types of data
 *
 * Outputs:
 * - 0: Same as input data
 */
class FAST_EXPORT Interleave : public Streamer {
    FAST_OBJECT(Interleave)
    public:
        /**
         * @brief Create instance
         * @param framerate Framerate to switch between input data
         * @return instance
         */
        FAST_CONSTRUCTOR(Interleave,
            int, framerate, = -1
        )
        void setFramerate(int framerate);
        void loadAttributes() override;
        void generateStream() override;
        ~Interleave();
    private:
        void execute();
        int m_framerate;
};

/**
 * @brief Interleaves input data streams into one output stream with playback control
 *
 * The difference between this InterleavePlayback and Interleave, is that this PO can be used with PlaybackWidget.
 *
 * Inputs:
 * - 0-N: Any types of data
 *
 * Outputs:
 * - 0: Same as input data
 */
class FAST_EXPORT InterleavePlayback : public RandomAccessStreamer {
    FAST_PROCESS_OBJECT(InterleavePlayback)
    public:
        /**
         * @brief Create instance
         * @param sourceStreamer The RandomAccessStreamer which is the source of this stream
         * @param framerate Framerate to switch between input data
         * @return instance
         */
        FAST_CONSTRUCTOR(InterleavePlayback,
                         std::shared_ptr<RandomAccessStreamer>, sourceStreamer,,
                         int, framerate, = -1)
        int getNrOfFrames() override;
        void generateStream() override;
        void setCurrentFrameIndex(int index) override;
        int getCurrentFrameIndex() override;
        void setFramerate(int framerate) override;
        int getFramerate() const override;
        void setPause(bool pause) override;
        void setLooping(bool loop) override;
    private:
        InterleavePlayback() { throw NotImplementedException(); };
        void execute();
        int m_nrOfConnections = 1;
        std::shared_ptr<RandomAccessStreamer> m_sourceStreamer;
};
}