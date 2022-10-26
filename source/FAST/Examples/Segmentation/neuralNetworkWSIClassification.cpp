/**
 * @example neuralNetworkWSIClassification.cpp
 */
#include <FAST/Tools/CommandLineParser.hpp>
#include <FAST/Algorithms/NeuralNetwork/InferenceEngineManager.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Visualization/HeatmapRenderer/HeatmapRenderer.hpp>
#include <FAST/Importers/WholeSlideImageImporter.hpp>
#include <FAST/Visualization/ImagePyramidRenderer/ImagePyramidRenderer.hpp>
#include <FAST/Algorithms/ImagePatch/PatchGenerator.hpp>
#include <FAST/Algorithms/ImagePatch/PatchStitcher.hpp>
#include <FAST/Algorithms/NeuralNetwork/SegmentationNetwork.hpp>
#include <FAST/Algorithms/TissueSegmentation/TissueSegmentation.hpp>

using namespace fast;

int main(int argc, char** argv) {
    CommandLineParser parser("Neural network WSI classification example");
    parser.addChoice("inference-engine",
            {"default", "OpenVINO", "TensorFlow", "TensorRT", "ONNXRuntime"},
            "default",
            "Which neural network inference engine to use");
    parser.addPositionVariable(1,
            "filename",
            Config::getTestDataPath() + "/WSI/A05.svs",
            "WSI to process");
    parser.parse(argc, argv);

    auto importer = WholeSlideImageImporter::create(parser.get("filename"));

    auto tissueSegmentation = TissueSegmentation::create()->connect(importer);

    auto generator = PatchGenerator::create(512, 512, 1, 0)
            ->connect(importer)
            ->connect(1, tissueSegmentation);

    InferenceEngine::pointer engine;
    if(parser.get("inference-engine") == "default") {
        engine = InferenceEngineManager::loadBestAvailableEngine();
    } else {
        engine = InferenceEngineManager::loadEngine(parser.get("inference-engine"));
    }
    auto network = NeuralNetwork::create(
            Config::getTestDataPath() + "NeuralNetworkModels/wsi_classification." + getModelFileExtension(engine->getPreferredModelFormat()),
            {}, {}, engine->getName())
        ->connect(generator);
    network->setScaleFactor(1.0f / 255.0f);

    auto stitcher = PatchStitcher::create()
            ->connect(network);

    auto renderer = ImagePyramidRenderer::create()
            ->connect(importer);

    auto heatmapRenderer = HeatmapRenderer::create(true, false, 0.5, 0.3)
            ->connect(stitcher);

    auto window = SimpleWindow2D::create()
            ->connect({renderer, heatmapRenderer});
    window->enableMaximized();
    window->run();
}
