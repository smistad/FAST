/**
 * @example neuralNetworkCTSegmentation.cpp
 */
#include <FAST/Tools/CommandLineParser.hpp>
#include <FAST/Importers/ImageFileImporter.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Algorithms/ImagePatch/PatchGenerator.hpp>
#include <FAST/Algorithms/ImagePatch/PatchStitcher.hpp>
#include <FAST/Algorithms/NeuralNetwork/SegmentationNetwork.hpp>
#include <FAST/Algorithms/TissueSegmentation/TissueSegmentation.hpp>
#include <FAST/Visualization/VolumeRenderer/AlphaBlendingVolumeRenderer.hpp>
#include <FAST/Visualization/VolumeRenderer/ThresholdVolumeRenderer.hpp>
#include <FAST/Algorithms/NeuralNetwork/InferenceEngineManager.hpp>

using namespace fast;

int main(int argc, char** argv) {
    CommandLineParser parser("Neural network CT volume segmentation example");
    parser.addChoice("inference-engine",
            {"default", "TensorFlow", "OpenVINO", "TensorRT"},
            "default",
            "Which neural network inference engine to use");
    parser.addPositionVariable(1,
            "filename",
            Config::getTestDataPath() + "/CT/LIDC-IDRI-0072/000001.dcm",
            "CT volume to process");
    parser.parse(argc, argv);

    auto importer = ImageFileImporter::create(parser.get("filename"));

    auto generator = PatchGenerator::create(512, 512, 32)
            ->connect(importer);
    generator->enableRuntimeMeasurements();

    auto network = SegmentationNetwork::New();
    if(parser.get("inference-engine") != "default")
        network->setInferenceEngine(parser.get("inference-engine"));
    const auto engine = network->getInferenceEngine()->getName();
    network->load(Config::getTestDataPath() + "/NeuralNetworkModels/lung_nodule_segmentation." + getModelFileExtension(network->getInferenceEngine()->getPreferredModelFormat()));
    network->setMinAndMaxIntensity(-1200.0f, 400.0f);
    network->setScaleFactor(1.0f / (400 + 1200));
    network->setMeanAndStandardDeviation(-1200.0f, 1.0f);
    network->setInputConnection(generator->getOutputPort());
    network->setResizeBackToOriginalSize(true);
    network->setThreshold(0.3);
    network->enableRuntimeMeasurements();

    auto stitcher = PatchStitcher::create()->connect(network);
    stitcher->enableRuntimeMeasurements();

    auto renderer = AlphaBlendingVolumeRenderer::create(TransferFunction::CT_Blood_And_Bone())
            ->connect(importer);

    auto renderer2 = ThresholdVolumeRenderer::create()->connect(stitcher);

    auto window = SimpleWindow3D::create()->connect({renderer, renderer2});
    window->run();
}
