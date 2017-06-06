#ifndef STREAMER_HPP_
#define STREAMER_HPP_

#include "FAST/SmartPointers.hpp"
#include "FAST/Data/DataTypes.hpp"
#include "FAST/Exception.hpp"

namespace fast {

enum StreamingMode { STREAMING_MODE_NEWEST_FRAME_ONLY, STREAMING_MODE_STORE_ALL_FRAMES, STREAMING_MODE_PROCESS_ALL_FRAMES };

class FAST_EXPORT  NoMoreFramesException : public Exception {
    public:
        NoMoreFramesException(std::string message) : Exception(message) {};
};

class FAST_EXPORT  Streamer {
    public:
        typedef SharedPointer<Streamer> pointer;
        virtual ~Streamer() {};
        virtual void setStreamingMode(StreamingMode mode);
        virtual StreamingMode getStreamingMode() const;
        virtual bool hasReachedEnd() const = 0;
        virtual uint getNrOfFrames() const = 0;
        static std::string getStaticNameOfClass() {
            return "Streamer";
        }
    protected:
        StreamingMode mStreamingMode;
        Streamer();


};

} // end namespace fast

#endif /* STREAMER_HPP_ */
