#include <FAST/Config.hpp>
#include <FAST/Utility.hpp>
#include <FAST/Tools/CommandLineParser.hpp>

using namespace fast;

int main(int argc, char** argv) {
    CommandLineParser parser("Download test data for FAST");
    parser.addOption("force", "Download even if data already exists");
    parser.addVariable("destination", "", "Override destination path of the test data");
    parser.parse(argc, argv);

    std::string destination = Config::getTestDataPath();
    if(parser.gotValue("destination"))
        destination = parser.get("destination");

    if(!parser.getOption("force") && fileExists(destination + "/LICENSE.md")) {
        std::cout << "Test data already exists in " << destination << std::endl;
        std::cout << "Use option --force to download and overwrite." << std::endl;
        return 0;
    }

    downloadTestDataIfNotExists(destination, parser.getOption("force"));

    return 0;
}
