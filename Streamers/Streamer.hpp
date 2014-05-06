#ifndef STREAMER_HPP_
#define STREAMER_HPP_

#include "ProcessObject.hpp"
#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>

namespace fast {

enum StreamingMode { STREAMING_MODE_NEWEST_FRAME_ONLY, STREAMING_MODE_KEEP_ALL_FRAMES };

class Streamer : public ProcessObject {
    public:
        virtual void producerStream() = 0;
        virtual ~Streamer() {};
        void setStreamingMode(StreamingMode mode);
        StreamingMode getStreamingMode() const;
    protected:
        StreamingMode mStreamingMode;
        Streamer();


};

} // end namespace fast

#endif /* STREAMER_HPP_ */
