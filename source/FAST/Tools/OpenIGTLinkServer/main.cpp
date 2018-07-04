#include <FAST/Tools/CommandLineParser.hpp>
#include "FAST/Streamers/Tests/DummyIGTLServer.hpp"
#include "GUI.hpp"

using namespace fast;

int main(int argc, char** argv) {
    CommandLineParser parser("OpenIGTLink Server");
    parser.addPositionVariable(1, "path", false, "Path to stream. Example: /path/to/stream/image_#.mhd;/path/to/second_stream/image_#.mhd");
    parser.addVariable("fps", false, "Set frames per second. Example: --fps 30");
    parser.parse(argc, argv);

    GUI::pointer window = GUI::New();

    if(parser.gotValue("path")) {
        std::vector<std::string> paths = split(parser.get("path"), ";");
        window->setFilenameFormats(paths);
    }
    if(parser.gotValue("fps")) {
        window->setFramesPerSecond(parser.get<int>("fps"));
    }

    Reporter::setGlobalReportMethod(Reporter::COUT);
    window->getReporter().setReportMethod(Reporter::COUT);
    window->start();
}