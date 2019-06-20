#include <FAST/Algorithms/BlockMatching/BlockMatching.hpp>
#include <FAST/Streamers/ImageFileStreamer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Visualization/VectorFieldRenderer/VectorFieldRenderer.hpp>
#include <FAST/Visualization/VectorFieldRenderer/VectorFieldColorRenderer.hpp>
#include <FAST/Tools/CommandLineParser.hpp>
#include <FAST/Algorithms/VectorMedianFilter/VectorMedianFilter.hpp>
#include <FAST/Exporters/ImageFileExporter.hpp>

using namespace fast;

int main(int argc, char** argv) {
    CommandLineParser parser("Block matching");
    parser.addPositionVariable(1, "path", Config::getTestDataPath() + "/US/Heart/ApicalFourChamber/US-2D_#.mhd", 
        "Path to the recording to stream, e.g. /path/to/frames/frame_#.mhd");
    parser.addVariable("block-size", "5", "The size in pixels of the blocks to match. Has to be odd.");
    parser.addVariable("search-size", "9", "The size in pixels of the grid to search for a match. Has to be odd");
    parser.addVariable("intensity-threshold", "75.0f", "Lower intensity of a threshold to search for a block. If a block has lower intensity that this, it will not look for a match.");
    parser.addVariable("median-filter-size", "7", "Size of vector median filter window to run after block matching. Must be odd and larger than 3.");
    parser.addVariable("export-to", "", "Specify a path to export the vector field for each frame. E.g. /path/to/somewhere/");
    parser.addChoice("export-format", {"mhd"}, "mhd", "Specify export format");
    parser.addChoice("matching-metric", {"SAD", "SSD", "NCC"}, "SAD", "Matching metric used for calculating how similar two blocks are.");
    parser.addOption("display-lines", "Display vector field as lines instead of color overlay");
    parser.addOption("disable-compression", "Disable compression when saving as mhd (.zraw)");
    parser.parse(argc, argv);

    auto streamer = ImageFileStreamer::New();
    streamer->setFilenameFormat(parser.get("path"));

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

    if(!parser.gotValue("export-to")) {
        streamer->enableLooping();
        // Visualize
        auto renderer = ImageRenderer::New();
        renderer->addInputConnection(streamer->getOutputPort());

        Renderer::pointer vectorRenderer;
        if (parser.getOption("display-lines")) {
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
    } else {
        // Export vector fields to disk
        std::string exportPath = parser.get("export-to");
        int timestep = 0;
        ImageFileExporter::pointer exporter = ImageFileExporter::New();
        exporter->setCompression(!parser.getOption("disable-compression"));
        exporter->setInputConnection(source->getOutputPort());
        while(streamer->getNrOfFrames() > timestep) {
            std::cout << "Processing frame: " << timestep << std::endl;
            const std::string path = join(exportPath, "frame_" + std::to_string(timestep) + "." + parser.get("export-format"));
            exporter->setFilename(path);
            exporter->update(timestep, STREAMING_MODE_PROCESS_ALL_FRAMES);
            ++timestep;
        }
    }
    blockMatching->getRuntime()->print();
}