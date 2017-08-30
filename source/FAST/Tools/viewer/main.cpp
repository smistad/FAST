#include "GUI.hpp"

using namespace fast;

int main() {
    Reporter::setGlobalReportMethod(Reporter::COUT);
    GUI::pointer window = GUI::New();
    window->start(STREAMING_MODE_STORE_ALL_FRAMES);

}