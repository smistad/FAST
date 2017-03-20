#include <FAST/Visualization/DualViewWindow.hpp>
#include <FAST/Visualization/HeatmapRenderer/HeatmapRenderer.hpp>
#include <FAST/Algorithms/ImageResampler/ImageResampler.hpp>
#include "FAST/Testing.hpp"
#include "FAST/Streamers/ImageFileStreamer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/MeshRenderer/MeshRenderer.hpp"
#include "FAST/Algorithms/NeuralNetwork/PixelClassification.hpp"

using namespace fast;

int main() {
    Reporter::setGlobalReportMethod(Reporter::COUT);
    ImageFileStreamer::pointer streamer = ImageFileStreamer::New();
    streamer->setFilenameFormats({
         "/home/smistad/data/eyeguide/axillary_nerve_block/17/2017Feb13_150433/#.png",
         "/home/smistad/data/eyeguide/axillary_nerve_block/17/2017Feb13_150648/#.png",
         "/home/smistad/data/eyeguide/axillary_nerve_block/17/2017Feb13_150824/#.png",
                                 });
    streamer->enableLooping();
    streamer->setStartNumber(1);
    streamer->setSleepTime(50);
    streamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);

    PixelClassification::pointer segmentation = PixelClassification::New();
    segmentation->setNrOfClasses(6);
    segmentation->load("/home/smistad/workspace/eyeguide_keras/models/network_graph.pb");
    segmentation->setInputSize(256, 256);
    segmentation->setScaleFactor(1.0f/255.0f);
    segmentation->setOutputParameters({"Reshape_21"});
    segmentation->setInputConnection(streamer->getOutputPort());
    segmentation->setHeatmapOutput();
    segmentation->enableRuntimeMeasurements();

    HeatmapRenderer::pointer renderer = HeatmapRenderer::New();
    renderer->addInputConnection(segmentation->getOutputPort(1), Color::Red());
    renderer->addInputConnection(segmentation->getOutputPort(2), Color::Blue());
    renderer->addInputConnection(segmentation->getOutputPort(3), Color::Green());
    renderer->addInputConnection(segmentation->getOutputPort(4), Color::Purple());
    renderer->addInputConnection(segmentation->getOutputPort(5), Color::Cyan());

    ImageRenderer::pointer renderer2 = ImageRenderer::New();
    renderer2->setInputConnection(streamer->getOutputPort());

    SimpleWindow::pointer window = SimpleWindow::New();

    window->addRenderer(renderer2);
    window->addRenderer(renderer);
    window->setSize(2560, 1440);
    window->getView()->set2DPixelSpacing(0.3);
    window->set2DMode();
    window->getView()->setBackgroundColor(Color::Black());
    window->start();

    segmentation->getRuntime()->print();
    segmentation->getRuntime("input_data_copy")->print();
    segmentation->getRuntime("network_execution")->print();
}
