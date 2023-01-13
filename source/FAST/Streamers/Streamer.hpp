#pragma once

#include <FAST/ProcessObject.hpp>
#include "FAST/Data/DataTypes.hpp"
#include "FAST/Exception.hpp"
#include <thread>

namespace fast {

/**
 * @brief Defines the streaming mode for a streamer
 */
enum class StreamingMode {
    NewestFrameOnly,
    ProcessAllFrames,
    StoreAllFrames
};

class FAST_EXPORT  NoMoreFramesException : public Exception {
    public:
        explicit NoMoreFramesException(std::string message) : Exception(message) {};
};

/**
 * @defgroup streamers Streamers
 * Process objects which produces a stream of data asynchronously. Must inherit from Streamer.
 */
/**
 * @brief Abstract base class for all @ref streamers
 *
 * All @ref streamers must inherit from this class.
 * @ingroup streamers
 */
class FAST_EXPORT Streamer : public ProcessObject {
    public:
        typedef std::shared_ptr<Streamer> pointer;
        Streamer();
        virtual ~Streamer() { stop(); };
        static std::string getStaticNameOfClass() {
            return "Streamer";
        }
        virtual std::string getNameOfClass() const override {
            return "Streamer";
        }

        /**
         * Stop the stream
         */
        virtual void stop();

        virtual bool isStopped();

        virtual void setMaximumNrOfFrames(int maximumNrOfFrames);

        void setStreamingMode(StreamingMode mode);
        StreamingMode getStreamingMode() const;

        virtual DataChannel::pointer getOutputPort(uint portID = 0) override;
    protected:
        /**
         * Block until the first data frame has been sent using a condition variable
         */
        virtual void waitForFirstFrame();

        /**
         * Starts the stream if it is not already started
         */
        virtual void startStream();

        /**
         * Signal that a frame was added. Necessary for unlocking the block in waitForFirstFrame
         */
        virtual void frameAdded();

        /**
         * The function producing the data stream
         */
        virtual void generateStream() = 0;

        bool m_firstFrameIsInserted = false;
        bool m_streamIsStarted = false;
        bool m_stop = false;
        StreamingMode m_streamingMode = StreamingMode::ProcessAllFrames;

        std::mutex m_firstFrameMutex;
        std::mutex m_stopMutex;
        std::unique_ptr<std::thread> m_thread;
        std::condition_variable m_firstFrameCondition;

        std::map<uint, std::shared_ptr<ProcessObject>> m_outputPOs;


};

} // end namespace fast
