/**
 * @exampel nonLocalMeans.cpp
 */
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

    auto streamer = ImageFileStreamer::create(parser.get("filename"), true);

    auto filter = NonLocalMeans::create(
            parser.get<int>("filter-size"),
            parser.get<int>("search-size"),
            parser.get<float>("smoothing")
            )->connect(streamer);
    filter->enableRuntimeMeasurements();
    filter->setPreProcess(!parser.getOption("disable-preprocessing"));

    auto enhance = UltrasoundImageEnhancement::create()->connect(filter);

    auto renderer = ImageRenderer::create()->connect(enhance);

    auto enhance2 = UltrasoundImageEnhancement::create()->connect(streamer);

    auto renderer2 = ImageRenderer::create()->connect(enhance2);

    auto window = DualViewWindow::New();
    window->set2DMode();
    window->addRendererToRightView(renderer);
    window->addRendererToLeftView(renderer2);
    window->getView(0)->setBackgroundColor(Color::Black());
    window->getView(1)->setBackgroundColor(Color::Black());
    window->run();

    // Print runtime of NLM
    filter->getRuntime()->print();
}