#include "TrackingGUI.hpp"

using namespace fast;

int main() {
    Config::setStreamingMode(STREAMING_MODE_NEWEST_FRAME_ONLY);
    auto window = TrackingGUI::New();
    window->start();
}