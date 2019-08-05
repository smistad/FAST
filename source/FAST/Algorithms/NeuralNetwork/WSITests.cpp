#include <FAST/Testing.hpp>
#include "InferenceEngineManager.hpp"
#include <FAST/Importers/ImageFileImporter.hpp>
#include <FAST/Visualization/SegmentationRenderer/SegmentationRenderer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Visualization/DualViewWindow.hpp>
#include <FAST/Algorithms/ImageCropper/ImageCropper.hpp>
#include <FAST/Streamers/ImageFileStreamer.hpp>
#include <FAST/Visualization/HeatmapRenderer/HeatmapRenderer.hpp>
#include <FAST/Importers/WholeSlideImageImporter.hpp>
#include <FAST/Visualization/ImagePyramidRenderer/ImagePyramidRenderer.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Data/ImagePyramid.hpp>
#include <FAST/Data/Image.hpp>
#include <FAST/Algorithms/ImagePatch/PatchGenerator.hpp>
#include <FAST/Algorithms/ImagePatch/PatchStitcher.hpp>
#include <FAST/Algorithms/ImagePatch/ImageToBatchGenerator.hpp>
#include <FAST/Algorithms/NeuralNetwork/NeuralNetwork.hpp>
#include <FAST/Algorithms/NeuralNetwork/ImageClassificationNetwork.hpp>
#include <FAST/Algorithms/NeuralNetwork/SegmentationNetwork.hpp>
#include <FAST/Algorithms/ImageChannelConverter/ImageChannelConverter.hpp>
#include <FAST/Algorithms/TissueSegmentation/TissueSegmentation.hpp>

using namespace fast;

TEST_CASE("WSI -> Patch generator -> Neural network -> Patch stitcher -> visualize", "[fast][neuralnetwork][wsi][visual]") {
    for(auto& engine : InferenceEngineManager::getEngineList()) {
        auto importer = WholeSlideImageImporter::New();
        importer->setFilename(Config::getTestDataPath() + "/WSI/A05.svs");

        auto tissueSegmentation = TissueSegmentation::New();
        tissueSegmentation->setInputConnection(importer->getOutputPort());

        auto generator = PatchGenerator::New();
        generator->setPatchSize(512, 512);
        generator->setPatchLevel(0);
        generator->setInputConnection(importer->getOutputPort());
        generator->setInputConnection(1, tissueSegmentation->getOutputPort());

        auto network = NeuralNetwork::New();
        network->setInferenceEngine(engine);
        if(engine.substr(0, 10) == "TensorFlow") {
            network->setOutputNode(0, "sequential/dense_1/Softmax", NodeType::TENSOR);
        } else if(engine == "TensorRT") {
            network->setInputNode(0, "input_1", NodeType::IMAGE, TensorShape{-1, 3, 512, 512});
            network->setOutputNode(0, "sequential/dense_1/Softmax", NodeType::TENSOR, TensorShape{-1, 3});
        }
        network->load(Config::getTestDataPath() + "NeuralNetworkModels/wsi_classification." +
                      network->getInferenceEngine()->getDefaultFileExtension());
        network->setInputConnection(generator->getOutputPort());
        network->setScaleFactor(1.0f / 255.0f);

        auto stitcher = PatchStitcher::New();
        stitcher->setInputConnection(network->getOutputPort());

        auto renderer = ImagePyramidRenderer::New();
        renderer->addInputConnection(importer->getOutputPort());

        auto heatmapRenderer = HeatmapRenderer::New();
        heatmapRenderer->addInputConnection(stitcher->getOutputPort());
        //heatmapRenderer->setMinConfidence(0.5);
        heatmapRenderer->setMaxOpacity(0.3);

        auto window = SimpleWindow::New();
        window->addRenderer(renderer);
        window->addRenderer(heatmapRenderer);
        window->setTimeout(4000);
        window->enableMaximized();
        window->set2DMode();
        window->start();
    }
}

TEST_CASE("WSI -> Patch generator -> Image to batch generator -> Neural network -> Patch stitcher -> visualize", "[fast][neuralnetwork][wsi][visual][batch]") {
    for(auto& engine : InferenceEngineManager::getEngineList()) {
        auto importer = WholeSlideImageImporter::New();
        importer->setFilename(Config::getTestDataPath() + "/WSI/A05.svs");

        auto tissueSegmentation = TissueSegmentation::New();
        tissueSegmentation->setInputConnection(importer->getOutputPort());

        auto generator = PatchGenerator::New();
        generator->setPatchSize(512, 512);
        generator->setPatchLevel(0);
        generator->setInputConnection(importer->getOutputPort());
        generator->setInputConnection(1, tissueSegmentation->getOutputPort());

        auto batchGenerator = ImageToBatchGenerator::New();
        batchGenerator->setMaxBatchSize(32);
        batchGenerator->setInputConnection(generator->getOutputPort());

        auto network = NeuralNetwork::New();
        network->setInferenceEngine(engine);
        if(engine.substr(0, 10) == "TensorFlow") {
            network->setOutputNode(0, "sequential/dense_1/Softmax", NodeType::TENSOR);
        } else if(engine == "TensorRT") {
            network->setInputNode(0, "input_1", NodeType::IMAGE, TensorShape{-1, 3, 512, 512});
            network->setOutputNode(0, "sequential/dense_1/Softmax", NodeType::TENSOR, TensorShape{-1, 3});
        }
        network->load(Config::getTestDataPath() + "NeuralNetworkModels/wsi_classification." +
                      network->getInferenceEngine()->getDefaultFileExtension());
        network->setInputConnection(batchGenerator->getOutputPort());
        network->setScaleFactor(1.0f / 255.0f);

        auto stitcher = PatchStitcher::New();
        stitcher->setInputConnection(network->getOutputPort());

        auto renderer = ImagePyramidRenderer::New();
        renderer->addInputConnection(importer->getOutputPort());

        auto heatmapRenderer = HeatmapRenderer::New();
        heatmapRenderer->addInputConnection(stitcher->getOutputPort());
        //heatmapRenderer->setMinConfidence(0.5);
        heatmapRenderer->setMaxOpacity(0.4);

        auto window = SimpleWindow::New();
        window->addRenderer(renderer);
        window->addRenderer(heatmapRenderer);
        window->setTimeout(4000);
        window->set2DMode();
        window->start();
    }
}

/*
TEST_CASE("WSI -> Patch generator -> Pixel classifier -> visualize", "[fast][neuralnetwork][wsi][visual]") {
    Config::setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);
    auto importer = WholeSlideImageImporter::New();
    importer->setFilename(Config::getTestDataPath() + "/WSI/A05.svs");

    auto tissueSegmentation = TissueSegmentation::New();
    tissueSegmentation->setInputConnection(importer->getOutputPort());

    auto generator = PatchGenerator::New();
    generator->setPatchSize(256, 256);
    generator->setPatchLevel(0);
    generator->setInputConnection(importer->getOutputPort());
    generator->setInputConnection(1, tissueSegmentation->getOutputPort());

    auto network = SegmentationNetwork::New();
    network->setInferenceEngine("TensorFlowCUDA");
    auto engine = network->getInferenceEngine()->getName();
    if(engine.substr(0, 10) == "TensorFlow") {
        network->load(Config::getTestDataPath() + "NeuralNetworkModels/nuclei_256_rgb_test.pb");
        network->setOutputNode(0, "conv2d_30/truediv");
    }
    network->setInputConnection(generator->getOutputPort());
    network->setScaleFactor(1.0f/255.0f);

    auto renderer = ImageRenderer::New();
    renderer->addInputConnection(generator->getOutputPort());

    auto segRenderer = SegmentationRenderer::New();
    segRenderer->addInputConnection(network->getOutputPort());
    segRenderer->setOpacity(0.5);

    auto heatmapRenderer = HeatmapRenderer::New();
    heatmapRenderer->addInputConnection(network->getOutputPort());

    auto window = SimpleWindow::New();
    window->addRenderer(renderer);
    //window->addRenderer(heatmapRenderer);
    window->addRenderer(segRenderer);
    window->setTimeout(3000);
    window->set2DMode();
    window->start();
}
 */
