#include <FAST/Tools/CommandLineParser.hpp>
#include <FAST/Pipeline.hpp>
#include "GUI.hpp"

using namespace fast;

int main(int argc, char** argv) {
    CommandLineParser parser("FAST Pipeline Executor", "Use this tool to execute pipelines described in text files", true);
    parser.addPositionVariable(1, "pipeline-filename", false, "Pipeline filename");
    parser.addVariable("datahub", false, "Download and run a pipeline from DataHub by specifying the item id. Example: --datahub item-unique-id");

    parser.parse(argc, argv);

    if(Config::getVisualization()) {
        auto gui = GUI::New();
        if(parser.gotValue("pipeline-filename")) {
            gui->setPipelineFile(parser.get("pipeline-filename"), parser.getVariables());
        } else if(parser.gotValue("datahub")) {
            DataHub hub;
            hub.download(parser.get("datahub"));
            gui->setPipelineFile(join(hub.getStorageDirectory(), parser.get("datahub"), "pipeline.fpl"), parser.getVariables());
        }
        gui->run();
    } else {
        Reporter::info() << "Running pipeline in headless mode" << Reporter::end();
        // Headless mode
        std::string filename;
        if(parser.gotValue("pipeline-filename")) {
            filename = parser.get("pipeline-filename");
        } else if(parser.gotValue("datahub")) {
            DataHub hub;
            hub.download(parser.get("datahub"));
            filename = join(hub.getStorageDirectory(), parser.get("datahub"), "pipeline.fpl");
        } else {
            throw Exception("You must supply a pipeline filename or datahub item id");
        }
        auto pipeline = Pipeline(filename, parser.getVariables());
        pipeline.run({}, {}, false);
    }
}