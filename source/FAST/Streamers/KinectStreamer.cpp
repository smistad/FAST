#include "KinectStreamer.hpp"
#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/frame_listener_impl.h>
#include <libfreenect2/registration.h>
#include "FAST/Data/Image.hpp"

namespace fast {

KinectStreamer::KinectStreamer() {
    createOutputPort<Image>(0, OUTPUT_DYNAMIC); // RGB
    createOutputPort<Image>(1, OUTPUT_DYNAMIC); // Depth image
}

void KinectStreamer::execute() {

}

void KinectStreamer::producerStream() {

}

bool KinectStreamer::hasReachedEnd() const {

}

uint KinectStreamer::getNrOfFrames() const {

}

}