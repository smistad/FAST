#include <FAST/Tools/CommandLineParser.hpp>
#include <FAST/Streamers/ImageFileStreamer.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Visualization/DualViewWindow.hpp>
#include <FAST/Algorithms/UltrasoundImageEnhancement/UltrasoundImageEnhancement.hpp>
#include <FAST/Algorithms/NonLocalMeans/NonLocalMeans.hpp>

using namespace fast;

int main(int argc, char** argv) {

    CommandLineParser parser("Non local means filtering",
            "This example performs speckle reduction using the multiscale non-local means algorithm.");
    parser.addPositionVariable(1, "filename", Config::getTestDataPath() + "US/Heart/ApicalFourChamber/US-2D_#.mhd");
    parser.addVariable("search-size", "11", "Search size of NLM");
    parser.addVariable("filter-size", "3", "Filter size of NLM");
    parser.addVariable("smoothing", "0.12", "Smoothing amount (paramter h in the NLM algorithmn)");
    parser.addOption("disable-preprocessing", "Disable median preprocessing useful for ultrasound images.");
    parser.parse(argc, argv);

    auto streamer = ImageFileStreamer::New();
    streamer->setFilenameFormat(parser.get("filename"));
    streamer->enableLooping();

    auto filter = NonLocalMeans::New();
    filter->setInputConnection(streamer->getOutputPort());
    filter->enableRuntimeMeasurements();
    filter->setSearchSize(parser.get<int>("search-size"));
    filter->setFilterSize(parser.get<int>("filter-size"));
    filter->setSmoothingAmount(parser.get<float>("smoothing"));
    filter->setPreProcess(!parser.getOption("disable-preprocessing"));

    auto enhance = UltrasoundImageEnhancement::New();
    enhance->setInputConnection(filter->getOutputPort());

    auto renderer = ImageRenderer::New();
    renderer->addInputConnection(enhance->getOutputPort());

    auto enhance2 = UltrasoundImageEnhancement::New();
    enhance2->setInputConnection(streamer->getOutputPort());

    auto renderer2 = ImageRenderer::New();
    renderer2->addInputConnection(enhance2->getOutputPort());

    auto window = DualViewWindow::New();
    window->addRendererToBottomRightView(renderer);
    window->addRendererToTopLeftView(renderer2);
    window->getView(0)->setBackgroundColor(Color::Black());
    window->getView(1)->setBackgroundColor(Color::Black());
    window->getView(0)->set2DMode();
    window->getView(1)->set2DMode();
    window->start();
    filter->getRuntime()->print();
}