#include "Streamer.hpp"
using namespace fast;

void Streamer::setStreamingMode(StreamingMode mode) {
    mStreamingMode = mode;
}

StreamingMode Streamer::getStreamingMode() const {
    return mStreamingMode;
}

Streamer::Streamer() {
    mStreamingMode = STREAMING_MODE_NEWEST_FRAME_ONLY;
}
