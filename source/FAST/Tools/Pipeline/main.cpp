#include <FAST/Tools/CommandLineParser.hpp>
#include <FAST/Pipeline.hpp>
#include <FAST/Visualization/MultiViewWindow.hpp>

using namespace fast;

int main(int argc, char** argv) {
    Reporter::setGlobalReportMethod(Reporter::COUT);
    CommandLineParser parser("FAST Pipeline Executor");
    parser.addPositionVariable(1, "filename", true, "Pipeline filename");

    parser.parse(argc, argv);

    auto pipeline = Pipeline("", "", parser.get("filename"));
    pipeline.parsePipelineFile();

    auto window = MultiViewWindow::New();
    for(auto view : pipeline.setup()) {
        window->addView(view);
    }
    window->start();
}