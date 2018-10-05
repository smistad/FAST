#include "GUI.hpp"

using namespace fast;

int main() {
    GUI::pointer window = GUI::New();
    window->start(STREAMING_MODE_NEWEST_FRAME_ONLY);
}