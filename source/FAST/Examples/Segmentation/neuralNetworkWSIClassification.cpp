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
            {"default", "OpenVINO", "TensorFlow", "TensorRT"},
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

    auto network = NeuralNetwork::New();
    if(parser.get("inference-engine") != "default")
        network->setInferenceEngine(parser.get("inference-engine"));
    const auto engine = network->getInferenceEngine()->getName();
    network->setInferenceEngine(engine);
    network->load(Config::getTestDataPath() + "NeuralNetworkModels/wsi_classification." + getModelFileExtension(network->getInferenceEngine()->getPreferredModelFormat()));
    network->setInputConnection(generator->getOutputPort());
    network->setScaleFactor(1.0f / 255.0f);

    auto stitcher = PatchStitcher::create()->connect(network);

    auto renderer = ImagePyramidRenderer::create()->connect(importer);

    auto heatmapRenderer = HeatmapRenderer::create()->connect(stitcher);
    heatmapRenderer->setMaxOpacity(0.3);

    auto window = SimpleWindow2D::create()
            ->connect({renderer, heatmapRenderer});
    window->enableMaximized();
    window->run();
}
