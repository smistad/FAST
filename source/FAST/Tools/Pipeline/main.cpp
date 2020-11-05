#include <FAST/Tools/CommandLineParser.hpp>
#include <FAST/Pipeline.hpp>
#include "GUI.hpp"

using namespace fast;

int main(int argc, char** argv) {
    CommandLineParser parser("FAST Pipeline Executor", "Use this tool to execute pipelines described in text files", true);
    parser.addPositionVariable(1, "pipeline-filename", false, "Pipeline filename");

    parser.parse(argc, argv);

    auto gui = GUI::New();
    if(parser.gotValue("pipeline-filename"))
		gui->setPipelineFile(parser.get("pipeline-filename"), parser.getVariables());
    gui->start();
}