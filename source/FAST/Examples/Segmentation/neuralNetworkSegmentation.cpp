/**
 * Examples/Segmentation/neuralNetworkSegmentation.cpp
 *
 * If you edit this example, please also update the wiki and source code file in the repository.
 */
#include <FAST/Visualization/DualViewWindow.hpp>
#include <FAST/Visualization/SegmentationRenderer/SegmentationRenderer.hpp>
#include <FAST/Algorithms/ImageResampler/ImageResampler.hpp>
#include "FAST/Testing.hpp"
#include "FAST/Streamers/ImageFileStreamer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Algorithms/NeuralNetwork/PixelClassifier.hpp"

using namespace fast;

int main() {
    Reporter::setGlobalReportMethod(Reporter::COUT);

    ImageFileStreamer::pointer streamer = ImageFileStreamer::New();
    streamer->setFilenameFormat(Config::getTestDataPath() + "US/JugularVein/US-2D_#.mhd");
    streamer->setTimestampFilename(Config::getTestDataPath() + "US/JugularVein/timestamps.fts");
    streamer->enableLooping();

    PixelClassifier::pointer segmentation = PixelClassifier::New();
    segmentation->setNrOfClasses(3);
    segmentation->setScaleFactor(1.0f / 255.0f);
    const auto engine = segmentation->getInferenceEngine()->getName();
    if(engine == "TensorFlow") {
        // TensorFlow needs to know what the output node is called
        segmentation->addOutputNode(0, "conv2d_23/truediv");
    } else if(engine == "TensorRT") {
        // TensorRT needs to know everything about the input and output nodes
        segmentation->addInputNode(0, "input_image", NodeType::IMAGE, TensorShape({-1, 1, 256, 256}));
        segmentation->addOutputNode(0, "permute_2/transpose");
    }
    segmentation->load(join(Config::getTestDataPath(),
                            "NeuralNetworkModels/jugular_vein_segmentation." + segmentation->getInferenceEngine()->getDefaultFileExtension()));
    segmentation->setInputConnection(streamer->getOutputPort());
    segmentation->enableRuntimeMeasurements();

    SegmentationRenderer::pointer segmentationRenderer = SegmentationRenderer::New();
    segmentationRenderer->addInputConnection(segmentation->getOutputPort());
    segmentationRenderer->setOpacity(0.25);
    segmentationRenderer->setColor(Segmentation::LABEL_FOREGROUND, Color::Red());
    segmentationRenderer->setColor(Segmentation::LABEL_BLOOD, Color::Blue());

    ImageRenderer::pointer imageRenderer = ImageRenderer::New();
    imageRenderer->setInputConnection(streamer->getOutputPort());

    SimpleWindow::pointer window = SimpleWindow::New();
    window->addRenderer(imageRenderer);
    window->addRenderer(segmentationRenderer);
    window->set2DMode();
    window->getView()->setBackgroundColor(Color::Black());
    window->start();
    segmentation->getAllRuntimes()->printAll();
}
