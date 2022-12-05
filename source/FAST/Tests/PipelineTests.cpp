#include <FAST/Testing.hpp>
#include <FAST/Pipeline.hpp>
#include <FAST/Visualization/ComputationThread.hpp>

using namespace fast;

TEST_CASE("Pipeline output data", "[fast][pipeline]") {
    auto pipeline = Pipeline(Config::getPipelinePath() + "/wsi_patch_classification.fpl");
    pipeline.parse({}, {}, false);
    auto data = pipeline.getAllPipelineOutputData([](float progress) {
        std::cout << "Progress: " << 100*progress << "%" << std::endl;
    });
    std::cout << "Done" << std::endl;
}

TEST_CASE("Pipeline finished signal", "[fast][pipeline]") {
    auto pipeline = Pipeline(Config::getPipelinePath() + "/wsi_patch_classification.fpl");
    pipeline.parse({}, {}, true);

    auto thread = ComputationThread::create();
    thread->setPipeline(pipeline);

    bool doneSignaled = false;
    QObject::connect(thread.get(), &ComputationThread::pipelineFinished, [thread, &doneSignaled]() {
        // This is running in QThread, not in main thread..
        doneSignaled = true;
        std::cout << "DONE" << std::endl;
        thread->stopWithoutBlocking();
    });

    auto t = thread->start();
    t->wait(); // Wait until finished
    CHECK(doneSignaled);
}

TEST_CASE("Pipeline finished signal and get output data", "[fast][pipeline]") {
    auto pipeline = Pipeline(Config::getPipelinePath() + "/wsi_patch_classification.fpl");
    pipeline.parse({}, {}, true);

    auto thread = ComputationThread::create();
    thread->setPipeline(pipeline);

    bool doneSignaled = false;
    std::map<std::string, DataObject::pointer> outputData;
    QObject::connect(thread.get(), &ComputationThread::pipelineFinished, [thread, &doneSignaled, &pipeline, &outputData]() {
        // This is running in QThread, not in main thread..
        doneSignaled = true;
        std::cout << "DONE" << std::endl;
        thread->stopWithoutBlocking();
        outputData = pipeline.getAllPipelineOutputData();
    });

    auto t = thread->start();
    t->wait(); // Wait until finished
    CHECK(doneSignaled);
    CHECK(outputData.size() == 1);
}