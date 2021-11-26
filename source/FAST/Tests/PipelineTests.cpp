#include <FAST/Testing.hpp>
#include <FAST/Pipeline.hpp>

using namespace fast;

TEST_CASE("Pipeline output data", "[fast][pipeline]") {
    auto pipeline = Pipeline(Config::getPipelinePath() + "/wsi_patch_classification.fpl");
    pipeline.parse({}, {}, false);
    auto data = pipeline.getAllPipelineOutputData([](float progress) {
        std::cout << "Progress: " << 100*progress << "%" << std::endl;
    });
    std::cout << "Done" << std::endl;
}