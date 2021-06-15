/**
 * Examples/Segmentation/neuralNetworkUltrasoundSegmentation.cpp
 *
 * If you edit this example, please also update the wiki and source code file in the repository.
 */
#include <FAST/Tools/CommandLineParser.hpp>
#include <FAST/Algorithms/NeuralNetwork/SegmentationNetwork.hpp>
#include <FAST/Streamers/ImageFileStreamer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Visualization/SegmentationRenderer/SegmentationRenderer.hpp>
#include <FAST/Visualization/SegmentationLabelRenderer/SegmentationLabelRenderer.hpp>
#include <FAST/Streamers/ClariusStreamer.hpp>
#include <FAST/Streamers/OpenIGTLinkStreamer.hpp>
#include <FAST/Algorithms/UltrasoundImageCropper/UltrasoundImageCropper.hpp>

using namespace fast;

int main(int argc, char** argv) {
    //Reporter::setGlobalReportMethod(Reporter::COUT);
    CommandLineParser parser("Neural network segmentation example");
    parser.addChoice("inference-engine",
            {"default", "OpenVINO", "TensorFlow", "TensorRT"},
            "default",
            "Which neural network inference engine to use");
    parser.addVariable("filename", Config::getTestDataPath() + "US/JugularVein/US-2D_#.mhd", "Path to files to stream from disk");
    parser.addVariable("filename-timestamps", Config::getTestDataPath() + "US/JugularVein/timestamps.fts", "Path to a file with timestamps related to 'filename'");
    parser.addVariable("clarius-ip", false, "IP address of Clarius ultrasound scanner to stream from.");
    parser.addVariable("clarius-port", "5828", "Port of Clarius ultrasound scanner to stream from.");
    parser.addVariable("openigtlink-ip", false, "IP address of OpenIGTLink server to stream from.");
    parser.addVariable("openigtlink-port", "18944", "Port of OpenIGTLink server to stream from.");
    parser.parse(argc, argv);

    ProcessObject::pointer inputStream;

    if(parser.gotValue("clarius-ip")) {
        Config::setStreamingMode(STREAMING_MODE_NEWEST_FRAME_ONLY);
        auto streamer = ClariusStreamer::New();
        streamer->setConnectionAddress(parser.get("clarius-ip"));
        streamer->setConnectionPort(std::stoi(parser.get("clarius-port")));
        auto cropper = UltrasoundImageCropper::New();
        cropper->setInputConnection(streamer->getOutputPort());
        inputStream = cropper;
    } else if(parser.gotValue("openigtlink-ip")) {
        Config::setStreamingMode(STREAMING_MODE_NEWEST_FRAME_ONLY);
        auto streamer = OpenIGTLinkStreamer::New();
        streamer->setConnectionAddress(parser.get("openigtlink-ip"));
        streamer->setConnectionPort(std::stoi(parser.get("openigtlink-port")));
        inputStream = streamer;
    } else {
        auto streamer = ImageFileStreamer::New();
        streamer->setFilenameFormat(parser.get("filename"));
        streamer->setTimestampFilename(parser.get("filename-timestamps"));
        streamer->enableLooping();
        inputStream = streamer;
    }

    auto segmentation = SegmentationNetwork::New();
    segmentation->setScaleFactor(1.0f / 255.0f);
    if(parser.get("inference-engine") != "default")
        segmentation->setInferenceEngine(parser.get("inference-engine"));
    segmentation->load(join(Config::getTestDataPath(),
        "NeuralNetworkModels/jugular_vein_segmentation." + getModelFileExtension(segmentation->getInferenceEngine()->getPreferredModelFormat())));
    segmentation->setInputConnection(inputStream->getOutputPort());
    segmentation->enableRuntimeMeasurements();

    auto segmentationRenderer = SegmentationRenderer::New();
    segmentationRenderer->addInputConnection(segmentation->getOutputPort());
    segmentationRenderer->setOpacity(0.25);
    segmentationRenderer->setColor(1, Color::Red());
    segmentationRenderer->setColor(2, Color::Blue());

    auto labelRenderer = SegmentationLabelRenderer::New();
    labelRenderer->addInputConnection(segmentation->getOutputPort());
    labelRenderer->setAreaThreshold(10);
    labelRenderer->setLabelName(1, "Artery");
    labelRenderer->setColor(1, Color::Red());
    labelRenderer->setLabelName(2, "Vein");
    labelRenderer->setColor(2, Color::Blue());

    auto imageRenderer = ImageRenderer::New();
    imageRenderer->setInputConnection(inputStream->getOutputPort());

    auto window = SimpleWindow::New();
    window->addRenderer(imageRenderer);
    window->addRenderer(segmentationRenderer);
    window->addRenderer(labelRenderer);
    window->set2DMode();
    window->getView()->setBackgroundColor(Color::Black());
    window->start();
    segmentation->getAllRuntimes()->printAll();
}
