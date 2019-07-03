#include "KinectTrackingGUI.hpp"

using namespace fast;

int main() {
    Config::setStreamingMode(STREAMING_MODE_NEWEST_FRAME_ONLY);
    KinectTrackingGUI::pointer window = KinectTrackingGUI::New();
    window->start();
}