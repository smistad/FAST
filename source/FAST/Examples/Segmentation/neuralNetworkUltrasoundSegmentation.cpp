/**
 * @example neuralNetworkUltrasoundSegmentation.cpp
 */
#include <FAST/Tools/CommandLineParser.hpp>
#include <FAST/Algorithms/NeuralNetwork/SegmentationNetwork.hpp>
#include <FAST/Streamers/ImageFileStreamer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Visualization/SegmentationRenderer/SegmentationRenderer.hpp>
#include <FAST/Visualization/SegmentationLabelRenderer/SegmentationLabelRenderer.hpp>
#include <FAST/Algorithms/UltrasoundImageCropper/UltrasoundImageCropper.hpp>
#include <FAST/Algorithms/NeuralNetwork/InferenceEngineManager.hpp>
#include <FAST/Visualization/Widgets/PlaybackWidget.hpp>

using namespace fast;

int main(int argc, char** argv) {
    CommandLineParser parser("Neural network ultrasound segmentation example");
    parser.addChoice("inference-engine",
            {"default", "OpenVINO", "TensorFlow", "TensorRT", "ONNXRuntime"},
            "default",
            "Which neural network inference engine to use");
    parser.addVariable("filename", Config::getTestDataPath() + "US/JugularVein/US-2D_#.mhd", "Path to files to stream from disk");
    parser.addVariable("filename-timestamps", Config::getTestDataPath() + "US/JugularVein/timestamps.fts", "Path to a file with timestamps related to 'filename'");
    parser.parse(argc, argv);

    auto streamer = ImageFileStreamer::create(parser.get("filename"), true);
    streamer->setTimestampFilename(parser.get("filename-timestamps"));

    InferenceEngine::pointer engine;
    if(parser.get("inference-engine") == "default") {
        engine = InferenceEngineManager::loadBestAvailableEngine();
    } else {
        engine = InferenceEngineManager::loadEngine(parser.get("inference-engine"));
    }

    auto segmentation = SegmentationNetwork::create(
            join(Config::getTestDataPath(), "NeuralNetworkModels/jugular_vein_segmentation." + getModelFileExtension(engine->getPreferredModelFormat())),
            {}, {}, engine->getName())
        ->connect(streamer);
    segmentation->setScaleFactor(1.0f / 255.0f);
    segmentation->enableRuntimeMeasurements();

    auto segmentationRenderer = SegmentationRenderer::create({{1, Color::Red()}, {2, Color::Blue()}}, 0.25)
          ->connect(segmentation);

    auto labelRenderer = SegmentationLabelRenderer::create(
            {{1, "Artery"}, {2, "Vein"}},
            {{1, Color::Red()}, {2, Color::Blue()}},
            10)
        ->connect(segmentation);

    auto imageRenderer = ImageRenderer::create()
            ->connect(streamer);

    auto window = SimpleWindow2D::create(Color::Black())
            ->connect({imageRenderer, segmentationRenderer, labelRenderer})
            ->connect(new PlaybackWidget(streamer));
    window->run();
    segmentation->getAllRuntimes()->printAll();
}
