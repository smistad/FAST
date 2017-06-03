#include <FAST/Visualization/DualViewWindow.hpp>
#include <FAST/Visualization/HeatmapRenderer/HeatmapRenderer.hpp>
#include <FAST/Algorithms/ImageResampler/ImageResampler.hpp>
#include "FAST/Testing.hpp"
#include "FAST/Streamers/ImageFileStreamer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/TriangleRenderer/TriangleRenderer.hpp"
#include "FAST/Algorithms/NeuralNetwork/PixelClassifier.hpp"

using namespace fast;

int main() {
    Reporter::setGlobalReportMethod(Reporter::COUT);
    ImageFileStreamer::pointer streamer = ImageFileStreamer::New();
    streamer->setFilenameFormats({
         "/home/smistad/data/eyeguide/axillary_nerve_block/17/2017Feb13_150433/#.png",
         "/home/smistad/data/eyeguide/axillary_nerve_block/17/2017Feb13_150648/#.png",
         "/home/smistad/data/eyeguide/axillary_nerve_block/17/2017Feb13_150824/#.png",
         "/home/smistad/data/eyeguide/axillary_nerve_block/5/2016Dec30_082009/#.png",
         "/home/smistad/data/eyeguide/axillary_nerve_block/5/2016Dec30_082046/#.png",
         "/home/smistad/data/eyeguide/axillary_nerve_block/5/2016Dec30_082110/#.png",
    });
    streamer->enableLooping();
    streamer->setStartNumber(1);
    streamer->setSleepTime(50);
    streamer->setStreamingMode(STREAMING_MODE_PROCESS_ALL_FRAMES);

    PixelClassifier::pointer segmentation = PixelClassifier::New();
    segmentation->setNrOfClasses(6);
    segmentation->load("/home/smistad/workspace/eyeguide_keras/models/network_graph.pb");
    segmentation->setInputSize(256, 256);
    segmentation->setScaleFactor(1.0f/255.0f);
    segmentation->setOutputParameters({"Reshape_18"});
    segmentation->setInputConnection(streamer->getOutputPort());
    segmentation->setHeatmapOutput();
    segmentation->enableRuntimeMeasurements();

    HeatmapRenderer::pointer renderer = HeatmapRenderer::New();
    renderer->addInputConnection(segmentation->getOutputPort(1), Color::Red());
    renderer->addInputConnection(segmentation->getOutputPort(2), Color::Yellow());
    renderer->addInputConnection(segmentation->getOutputPort(3), Color::Green());
    renderer->addInputConnection(segmentation->getOutputPort(4), Color::Purple());
    renderer->addInputConnection(segmentation->getOutputPort(5), Color::Cyan());
    renderer->setMaxOpacity(0.2);
    renderer->setMinConfidence(0.25);
    renderer->enableRuntimeMeasurements();

    ImageRenderer::pointer renderer2 = ImageRenderer::New();
    renderer2->setInputConnection(streamer->getOutputPort());

    SimpleWindow::pointer window = SimpleWindow::New();

    window->addRenderer(renderer2);
    window->addRenderer(renderer);
    window->getView()->enableRuntimeMeasurements();
    window->setSize(1920, 1080);
    //window->enableFullscreen();
    window->getView()->set2DPixelSpacing(0.3);
    window->set2DMode();
    window->getView()->setBackgroundColor(Color::Black());
    window->start();

    segmentation->getAllRuntimes()->printAll();
    renderer->getAllRuntimes()->printAll();
    window->getView()->getAllRuntimes()->printAll();
}
