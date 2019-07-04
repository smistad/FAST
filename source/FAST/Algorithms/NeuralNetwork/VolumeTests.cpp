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
#include <FAST/Algorithms/NeuralNetwork/ImageClassifier.hpp>
#include <FAST/Algorithms/NeuralNetwork/PixelClassifier.hpp>
#include <FAST/Algorithms/ImageChannelConverter/ImageChannelConverter.hpp>
#include <FAST/Algorithms/TissueSegmentation/TissueSegmentation.hpp>
#include <FAST/Visualization/VolumeRenderer/AlphaBlendingVolumeRenderer.hpp>
#include <FAST/Visualization/VolumeRenderer/ThresholdVolumeRenderer.hpp>
#include <FAST/Algorithms/ImageResampler/ImageResampler.hpp>

using namespace fast;

TEST_CASE("Volume -> Patch generator -> Neural network -> Patch stitcher -> visualize", "[fast][neuralnetwork][volume][visual]") {

    auto importer = ImageFileImporter::New();
    importer->setFilename(Config::getTestDataPath() + "/CT/CT-Thorax.mhd");
    //importer->setFilename(Config::getTestDataPath() + "/CT/pasient09.mhd");

    /*
    auto resampler = ImageResampler::New();
    resampler->setOutputSpacing(0.5f, 0.5f, 0.5f);
    resampler->setInputConnection(importer->getOutputPort());
     */

    auto generator = PatchGenerator::New();
    generator->setPatchSize(512, 512, 64);
    generator->setInputConnection(importer->getOutputPort());

    auto network = PixelClassifier::New();
    network->setInferenceEngine("TensorFlowCUDA");
    network->load(Config::getTestDataPath() + "/NeuralNetworkModels/lung_nodule_model.pb");
    network->setMinAndMaxIntensity(-1200.0f, 400.0f);
    network->setScaleFactor(1.0f/(400+1200));
    network->setMeanAndStandardDeviation(-1200.0f, 1.0f);
    network->setOutputNode(0, "conv3d_18/truediv");
    network->setInputConnection(generator->getOutputPort());
    network->setResizeBackToOriginalSize(true);

    auto stitcher = PatchStitcher::New();
    stitcher->setInputConnection(network->getOutputPort());

    auto renderer = AlphaBlendingVolumeRenderer::New();
    renderer->setTransferFunction(TransferFunction::CT_Blood_And_Bone());
    renderer->addInputConnection(importer->getOutputPort());

    auto renderer2 = ThresholdVolumeRenderer::New();
    renderer2->addInputConnection(stitcher->getOutputPort());

    auto window = SimpleWindow::New();
    window->addRenderer(renderer2);
    window->addRenderer(renderer);
    //window->setTimeout(1000);
    window->start();
}

