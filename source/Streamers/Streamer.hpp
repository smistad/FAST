#ifndef STREAMER_HPP_
#define STREAMER_HPP_

#include "SmartPointers.hpp"

namespace fast {

enum StreamingMode { STREAMING_MODE_NEWEST_FRAME_ONLY, STREAMING_MODE_STORE_ALL_FRAMES, STREAMING_MODE_PROCESS_ALL_FRAMES };

class Streamer {
    public:
        typedef SharedPointer<Streamer> pointer;
        virtual void producerStream() = 0;
        virtual ~Streamer() {};
        void setStreamingMode(StreamingMode mode);
        StreamingMode getStreamingMode() const;
        virtual bool hasReachedEnd() const = 0;
    protected:
        StreamingMode mStreamingMode;
        Streamer();


};

} // end namespace fast

#endif /* STREAMER_HPP_ */
