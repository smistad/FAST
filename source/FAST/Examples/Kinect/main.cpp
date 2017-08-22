#include "KinectTrackingGUI.hpp"

using namespace fast;

int main() {
    KinectTrackingGUI::pointer window = KinectTrackingGUI::New();
    window->start(STREAMING_MODE_NEWEST_FRAME_ONLY);
}