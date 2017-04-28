#include "FAST/Streamers/Tests/DummyIGTLServer.hpp"
#include "GUI.hpp"

using namespace fast;

int main(int argc, char** argv) {
    if(argc > 1 && std::string(argv[1]) == "--help") {
        std::cout << "usage: " << argv[0] << " [/path/to/stream/image_#.mhd /path/to/second/stream/image_#.mhd ...]" << std::endl;
        return 0;
    }

    std::vector<std::string> paths;
    for(int i = 1; i < argc; ++i) {
        paths.push_back(std::string(argv[i]));
    }


    GUI::pointer window = GUI::New();
    window->getReporter().setReportMethod(Reporter::COUT);
    window->setFilenameFormats(paths);
    window->start();
}