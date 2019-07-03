#include "KinectTrackingGUI.hpp"

using namespace fast;

int main() {
    Config::setStreamingMode(STREAMING_MODE_NEWEST_FRAME_ONLY);
    auto window = KinectTrackingGUI::New();
    window->start();
}