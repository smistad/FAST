#include "GUI.hpp"

using namespace fast;

int main() {
    Config::setStreamingMode(STREAMING_MODE_NEWEST_FRAME_ONLY);
    //Reporter::setGlobalReportMethod(Reporter::COUT);
    auto window = GUI::New();
    window->start();

}