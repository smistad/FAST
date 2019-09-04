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
#include <FAST/Streamers/ClariusStreamer.hpp>
#include <FAST/Streamers/OpenIGTLinkStreamer.hpp>

using namespace fast;

int main(int argc, char** argv) {
    //Reporter::setGlobalReportMethod(Reporter::COUT);
    CommandLineParser parser("Neural network segmentation example");
    parser.addChoice("inference-engine",
            {"default", "OpenVINO", "TensorFlowCPU", "TensorFlowCUDA", "TensorRT", "TensorFlowROCm"},
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
        auto streamer = ClariusStreamer::New();
        streamer->setConnectionAddress(parser.get("clarius-ip"));
        streamer->setConnectionPort(std::stoi(parser.get("clarius-port")));
        inputStream = streamer;
    } else if(parser.gotValue("openigtlink-ip")) {
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
    if(parser.get("inference-engine") != "default") {
        segmentation->setInferenceEngine(parser.get("inference-engine"));
    }
    const auto engine = segmentation->getInferenceEngine()->getName();
    if(engine.substr(0,10) == "TensorFlow") {
        // TensorFlow needs to know what the output node is called
        segmentation->setOutputNode(0, "conv2d_23/truediv");
    } else if(engine == "TensorRT") {
        // TensorRT needs to know everything about the input and output nodes
        segmentation->setInputNode(0, "input_image", NodeType::IMAGE, TensorShape({-1, 1, 256, 256}));
        segmentation->setOutputNode(0, "permute_2/transpose", NodeType::TENSOR, TensorShape({-1, 3, 256, 256}));
    }
    segmentation->load(join(Config::getTestDataPath(),
                            "NeuralNetworkModels/jugular_vein_segmentation." + segmentation->getInferenceEngine()->getDefaultFileExtension()));
    segmentation->setInputConnection(inputStream->getOutputPort());
    segmentation->enableRuntimeMeasurements();

    auto segmentationRenderer = SegmentationRenderer::New();
    segmentationRenderer->addInputConnection(segmentation->getOutputPort());
    segmentationRenderer->setOpacity(0.25);
    segmentationRenderer->setColor(Segmentation::LABEL_FOREGROUND, Color::Red());
    segmentationRenderer->setColor(Segmentation::LABEL_BLOOD, Color::Blue());

    auto imageRenderer = ImageRenderer::New();
    imageRenderer->setInputConnection(inputStream->getOutputPort());

    auto window = SimpleWindow::New();
    window->addRenderer(imageRenderer);
    window->addRenderer(segmentationRenderer);
    window->set2DMode();
    window->getView()->setBackgroundColor(Color::Black());
    window->start();
    segmentation->getAllRuntimes()->printAll();
}
