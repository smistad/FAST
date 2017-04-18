#include "GUI.hpp"

using namespace fast;

int main() {
    Reporter::setGlobalReportMethod(Reporter::COUT);
    GUI::pointer window = GUI::New();
    window->start();

}