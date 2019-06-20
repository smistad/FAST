#include <FAST/Visualization/DualViewWindow.hpp>
#include <FAST/Visualization/HeatmapRenderer/HeatmapRenderer.hpp>
#include <FAST/Algorithms/ImageResampler/ImageResampler.hpp>
#include <FAST/Tools/CommandLineParser.hpp>
#include <FAST/Streamers/IGTLinkStreamer.hpp>
#include "FAST/Testing.hpp"
#include "FAST/Streamers/ImageFileStreamer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/TriangleRenderer/TriangleRenderer.hpp"
#include "FAST/Algorithms/NeuralNetwork/PixelClassifier.hpp"
#include <FAST/Algorithms/UltrasoundImageCropper/UltrasoundImageCropper.hpp>

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
        auto streamer = IGTLinkStreamer::New();
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
    PixelClassifier::pointer segmentation = PixelClassifier::New();
    const auto engine = segmentation->getInferenceEngine()->getName();
    if(engine == "TensorFlow") {
        segmentation->addOutputNode(0, "conv2d_23/truediv");
    } else if(engine == "TensorRT") {
        // TODO specify nodes
    }
    if(parser.get("model-filename") == "unspecified") {
        path += "." + segmentation->getInferenceEngine()->getDefaultFileExtension();
    } else {
        path = parser.get("model-filename");
    }
    segmentation->load(path);
    segmentation->setHeatmapOutput();
    segmentation->setNrOfClasses(6);
    segmentation->setScaleFactor(1.0f / 255.0f);
    segmentation->setInputConnection(inputStream->getOutputPort());
    //segmentation->setPreserveAspectRatio(true);
    segmentation->enableRuntimeMeasurements();
    segmentation->setHorizontalFlipping(parser.getOption("flip"));

    HeatmapRenderer::pointer renderer = HeatmapRenderer::New();
    renderer->addInputConnection(segmentation->getOutputPort(1), Color::Red());
    renderer->addInputConnection(segmentation->getOutputPort(2), Color::Yellow());
    renderer->addInputConnection(segmentation->getOutputPort(3), Color::Green());
    renderer->addInputConnection(segmentation->getOutputPort(4), Color::Magenta());
    renderer->addInputConnection(segmentation->getOutputPort(5), Color::Cyan());
    renderer->setMaxOpacity(0.6);
    //renderer->setMinConfidence(0.2);
    renderer->enableRuntimeMeasurements();

    ImageRenderer::pointer renderer2 = ImageRenderer::New();
    renderer2->setInputConnection(inputStream->getOutputPort());

    SimpleWindow::pointer window = SimpleWindow::New();

    window->addRenderer(renderer2);
    window->addRenderer(renderer);
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
