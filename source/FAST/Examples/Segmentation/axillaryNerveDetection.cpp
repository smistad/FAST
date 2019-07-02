#include <FAST/Visualization/DualViewWindow.hpp>
#include <FAST/Visualization/HeatmapRenderer/HeatmapRenderer.hpp>
#include <FAST/Algorithms/ImageResampler/ImageResampler.hpp>
#include <FAST/Tools/CommandLineParser.hpp>
#include <FAST/Streamers/OpenIGTLinkStreamer.hpp>
#include "FAST/Testing.hpp"
#include "FAST/Streamers/ImageFileStreamer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/TriangleRenderer/TriangleRenderer.hpp"
#include "FAST/Algorithms/NeuralNetwork/PixelClassifier.hpp"
#include <FAST/Algorithms/UltrasoundImageCropper/UltrasoundImageCropper.hpp>
#include <FAST/Visualization/SegmentationRenderer/SegmentationRenderer.hpp>

using namespace fast;

int main(int argc, char** argv) {
    Reporter::setGlobalReportMethod(Reporter::COUT);
    CommandLineParser parser("Axillary heatmap demo");
    parser.addVariable("model-filename", "unspecified");
    parser.addVariable("recording", Config::getTestDataPath() + "US/Axillary/US-2D_#.mhd");
    parser.addVariable("openigtlink-ip", false);
    parser.addOption("flip");
    parser.parse(argc, argv);

    ProcessObject::pointer inputStream;
    if(parser.gotValue("openigtlink-ip")) {
        auto streamer = OpenIGTLinkStreamer::New();
        streamer->setConnectionAddress(parser.get("openigtlink-ip"));
        auto cropper = UltrasoundImageCropper::New();
        cropper->setInputConnection(streamer->getOutputPort());
        inputStream = cropper;
    } else {
        auto streamer = ImageFileStreamer::New();
        streamer->setFilenameFormat(parser.get("recording"));
        streamer->setStartNumber(1);
        streamer->setSleepTime(50);
        inputStream = streamer;
    }

    std::string path = join(Config::getTestDataPath(), "NeuralNetworkModels/axillary_all_augmentations");
    auto segmentation = PixelClassifier::New();
    segmentation->setInferenceEngine("TensorFlowCPU");
    const auto engine = segmentation->getInferenceEngine()->getName();
    if(engine.substr(0, 10) == "TensorFlow") {
        segmentation->setOutputNode(0, "conv2d_23/truediv");
    } else if(engine == "TensorRT") {
        // TODO specify nodes
    }
    if(parser.get("model-filename") == "unspecified") {
        path += "." + segmentation->getInferenceEngine()->getDefaultFileExtension();
    } else {
        path = parser.get("model-filename");
    }
    segmentation->load(path);
    segmentation->setScaleFactor(1.0f / 255.0f);
    segmentation->setInputConnection(inputStream->getOutputPort());
    //segmentation->setPreserveAspectRatio(true);
    segmentation->enableRuntimeMeasurements();
    segmentation->setHorizontalFlipping(parser.getOption("flip"));
    segmentation->setHeatmapOutput();

    auto renderer = HeatmapRenderer::New();
    renderer->addInputConnection(segmentation->getOutputPort());
    renderer->setChannelColor(1, Color::Red());
    renderer->setChannelColor(2, Color::Yellow());
    renderer->setChannelColor(3, Color::Green());
    renderer->setChannelColor(4, Color::Magenta());
    renderer->setChannelColor(5, Color::Cyan());
    renderer->setMaxOpacity(0.6);
    //renderer->setMinConfidence(0.2);
    renderer->enableRuntimeMeasurements();

    /*
    auto segRenderer = SegmentationRenderer::New();
    segRenderer->addInputConnection(segmentation->getOutputPort());
    segRenderer->setOpacity(0.5);
     */

    auto renderer2 = ImageRenderer::New();
    renderer2->setInputConnection(inputStream->getOutputPort());

    auto window = SimpleWindow::New();

    window->addRenderer(renderer2);
    window->addRenderer(renderer);
    //window->addRenderer(segRenderer);
    window->getView()->enableRuntimeMeasurements();
    window->setSize(window->getScreenWidth(), window->getScreenHeight());
    window->enableMaximized();
    window->getView()->setAutoUpdateCamera(true);
    //window->enableFullscreen();
    window->set2DMode();
    window->getView()->setBackgroundColor(Color::Black());
    window->start();

    segmentation->getAllRuntimes()->printAll();
    renderer->getAllRuntimes()->printAll();
    window->getView()->getAllRuntimes()->printAll();
}
