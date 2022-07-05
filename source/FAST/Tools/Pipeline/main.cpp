#include <FAST/Tools/CommandLineParser.hpp>
#include <FAST/Pipeline.hpp>
#include "GUI.hpp"

using namespace fast;

int main(int argc, char** argv) {
    CommandLineParser parser("FAST Pipeline Executor", "Use this tool to execute pipelines described in text files", true);
    parser.addPositionVariable(1, "pipeline-filename", false, "Pipeline filename");
    parser.addVariable("datahub", false, "Download and run a pipeline from DataHub by specifying the item id. Example: --datahub item-unique-id");

    parser.parse(argc, argv);

    auto gui = GUI::New();
    if(parser.gotValue("pipeline-filename"))
		gui->setPipelineFile(parser.get("pipeline-filename"), parser.getVariables());
    if(parser.gotValue("datahub")) {
        DataHub hub;
        hub.download(parser.get("datahub"));
        gui->setPipelineFile(join(hub.getStorageDirectory(), parser.get("datahub"), "pipeline.fpl"), parser.getVariables());
    }
    gui->run();
}