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
#include <FAST/Visualization/VeryLargeImageRenderer/VeryLargeImageRenderer.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Data/WholeSlideImage.hpp>
#include <FAST/Data/Image.hpp>
#include <FAST/Algorithms/ImagePatch/PatchGenerator.hpp>
#include <FAST/Algorithms/ImagePatch/PatchStitcher.hpp>
#include <FAST/Algorithms/ImagePatch/ImageToBatchGenerator.hpp>
#include <FAST/Algorithms/NeuralNetwork/NeuralNetwork.hpp>
#include <FAST/Algorithms/NeuralNetwork/ImageClassifier.hpp>
#include <FAST/Algorithms/NeuralNetwork/PixelClassifier.hpp>
#include <FAST/Algorithms/ImageChannelConverter/ImageChannelConverter.hpp>

using namespace fast;

TEST_CASE("WSI -> Patch generator -> Neural network -> Patch stitcher -> visualize", "[fast][neuralnetwork][wsi][visual]") {
    auto importer = WholeSlideImageImporter::New();
    importer->setFilename(Config::getTestDataPath() + "/CMU-1.tiff");

    auto generator = PatchGenerator::New();
    generator->setPatchSize(512, 512);
    generator->setPatchLevel(2);
    generator->setInputConnection(importer->getOutputPort());

    auto converter = ImageChannelConverter::New();
    converter->setChannelsToRemove(false, true, true, false);
    converter->setInputConnection(generator->getOutputPort());

    auto classifier = NeuralNetwork::New();
    classifier->setInferenceEngine("OpenVINO");
    classifier->load(Config::getTestDataPath() + "NeuralNetworkModels/single_input_multi_output.xml");
    classifier->setInputConnection(converter->getOutputPort());

    auto stitcher = PatchStitcher::New();
    stitcher->setInputConnection(classifier->getOutputPort());

    auto renderer = VeryLargeImageRenderer::New();
    renderer->addInputConnection(importer->getOutputPort());

    auto heatmapRenderer = HeatmapRenderer::New();
    heatmapRenderer->addInputConnection(stitcher->getOutputPort());

    auto window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->addRenderer(heatmapRenderer);
    //window->setTimeout(1000);
    window->set2DMode();
    window->start();
}