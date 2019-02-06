#include <FAST/Algorithms/BlockMatching/BlockMatching.hpp>
#include <FAST/Streamers/ImageFileStreamer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Visualization/VectorFieldRenderer/VectorFieldRenderer.hpp>
#include <FAST/Visualization/VectorFieldRenderer/VectorFieldColorRenderer.hpp>
#include <FAST/Tools/CommandLineParser.hpp>
#include <FAST/Algorithms/VectorMedianFilter/VectorMedianFilter.hpp>

using namespace fast;

int main(int argc, char** argv) {
    CommandLineParser parser("Block matching");
    parser.addPositionVariable(1, "path", Config::getTestDataPath() + "/US/Heart/ApicalFourChamber/US-2D_#.mhd", 
        "Path to the recording to stream, e.g. /path/to/frames/frame_#.mhd");
    parser.addVariable("block-size", "5", "The size in pixels of the blocks to match. Has to be odd.");
    parser.addVariable("search-size", "9", "The size in pixels of the grid to search for a match. Has to be odd");
    parser.addVariable("intensity-threshold", "75.0f", "Lower intensity of a threshold to search for a block. If a block has lower intensity that this, it will not look for a match.");
    parser.addVariable("median-filter-size", "7", "Size of vector median filter window to run after block matching. Must be odd and larger than 3.");
    parser.addChoice("matching-metric", {"SAD", "SSD", "NCC"}, "SAD", "Matching metric used for calculating how similar two blocks are.");
    parser.addOption("display-lines", "Display vector field as lines instead of color overlay");
    parser.parse(argc, argv);

    auto streamer = ImageFileStreamer::New();
    streamer->setFilenameFormat(parser.get("path"));
    streamer->enableLooping();

    auto blockMatching = BlockMatching::New();
    blockMatching->setInputConnection(streamer->getOutputPort());
    blockMatching->setIntensityThreshold(parser.get<float>("intensity-threshold"));
    blockMatching->setBlockSize(parser.get<int>("block-size"));
    blockMatching->setSearchSize(parser.get<int>("search-size"));
    blockMatching->setMatchingMetric(BlockMatching::stringToMetric(parser.get("matching-metric")));
    blockMatching->enableRuntimeMeasurements();

    ProcessObject::pointer source = blockMatching;
    if(parser.get<int>("median-filter-size") > 0) {
        auto filter = VectorMedianFilter::New();
        filter->setWindowSize(parser.get<int>("median-filter-size"));
        filter->setInputConnection(blockMatching->getOutputPort());
        source = filter;
    }

    auto renderer = ImageRenderer::New();
    renderer->addInputConnection(streamer->getOutputPort());

    Renderer::pointer vectorRenderer;
    if(parser.getOption("display-lines")) {
        vectorRenderer = VectorFieldRenderer::New();
    } else {
        vectorRenderer = VectorFieldColorRenderer::New();
    }
    vectorRenderer->addInputConnection(source->getOutputPort());

    auto window = SimpleWindow::New();
    window->addRenderer(renderer);
    window->addRenderer(vectorRenderer);
    window->getView()->setBackgroundColor(Color::Black());
    window->set2DMode();
    window->start();
    blockMatching->getRuntime()->print();
}