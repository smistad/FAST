/**
 * Examples/Segmentation/neuralNetworkCTSegmentation.cpp
 *
 * If you edit this example, please also update the wiki and source code file in the repository.
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
    Reporter::setGlobalReportMethod(Reporter::COUT);
    CommandLineParser parser("Neural network CT volume segmentation example");
    parser.addChoice("inference-engine",
            {"default", "TensorFlowCPU", "TensorFlowCUDA", "TensorFlowROCm"},
            "default",
            "Which neural network inference engine to use");
    parser.addPositionVariable(1,
            "filename",
            Config::getTestDataPath() + "/CT/LIDC-IDRI-0072/000001.dcm",
            "CT volume to process");
    parser.parse(argc, argv);

    auto importer = ImageFileImporter::New();
    importer->setFilename(parser.get("filename"));

    auto generator = PatchGenerator::New();
    generator->setPatchSize(512, 512, 32);
    generator->setInputConnection(importer->getOutputPort());
    generator->enableRuntimeMeasurements();

    auto network = SegmentationNetwork::New();
    if(parser.get("inference-engine") != "default") {
        network->setInferenceEngine(parser.get("inference-engine"));
    } else {
        if(InferenceEngineManager::isEngineAvailable("TensorFlowCUDA")) {
            network->setInferenceEngine("TensorFlowCUDA");
        } else {
            network->setInferenceEngine("TensorFlowCPU");
        }
    }
    const auto engine = network->getInferenceEngine()->getName();
    network->load(Config::getTestDataPath() + "/NeuralNetworkModels/lung_nodule_segmentation.pb");
    network->setMinAndMaxIntensity(-1200.0f, 400.0f);
    network->setScaleFactor(1.0f / (400 + 1200));
    network->setMeanAndStandardDeviation(-1200.0f, 1.0f);
    network->setOutputNode(0, "conv3d_14/truediv");
    network->setInputConnection(generator->getOutputPort());
    network->setResizeBackToOriginalSize(true);
    network->setThreshold(0.3);
    network->enableRuntimeMeasurements();

    auto stitcher = PatchStitcher::New();
    stitcher->setInputConnection(network->getOutputPort());
    stitcher->enableRuntimeMeasurements();

    auto renderer = AlphaBlendingVolumeRenderer::New();
    renderer->setTransferFunction(TransferFunction::CT_Blood_And_Bone());
    renderer->addInputConnection(importer->getOutputPort());

    auto renderer2 = ThresholdVolumeRenderer::New();
    renderer2->addInputConnection(stitcher->getOutputPort());

    auto window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->addRenderer(renderer2);
    window->start();
}
