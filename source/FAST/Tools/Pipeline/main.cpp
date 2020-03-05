#include <FAST/Tools/CommandLineParser.hpp>
#include <FAST/Pipeline.hpp>
#include <FAST/Visualization/MultiViewWindow.hpp>

using namespace fast;

int main(int argc, char** argv) {
    
    CommandLineParser parser("FAST Pipeline Executor", "Use this tool to execute pipelines described in text files", true);
    parser.addPositionVariable(1, "pipeline-filename", true, "Pipeline filename");

    parser.parse(argc, argv);

    auto pipeline = Pipeline(parser.get("pipeline-filename"), parser.getVariables());
    pipeline.parsePipelineFile();

    auto window = MultiViewWindow::New();
    for(auto view : pipeline.getViews()) {
        window->addView(view);
    }
    window->start();
}