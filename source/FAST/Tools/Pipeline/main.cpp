#include <FAST/Tools/CommandLineParser.hpp>
#include <FAST/Pipeline.hpp>
#include <FAST/Visualization/MultiViewWindow.hpp>

using namespace fast;

int main(int argc, char** argv) {
    Reporter::setGlobalReportMethod(Reporter::COUT);
    CommandLineParser parser("FAST Pipeline Executor", "", true);
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