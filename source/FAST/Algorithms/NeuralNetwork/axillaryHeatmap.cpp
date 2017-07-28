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
    std::map<int, std::vector<std::string>> subjects;
    /*
    subjects[1] = {
            "/home/smistad/data/eyeguide/axillary_nerve_block/1/2016-10-07-135630/US-2D_#.mhd"
    };
    subjects[5] = {
            "/home/smistad/data/eyeguide/axillary_nerve_block/5/2016Dec30_082009/#.png",
            "/home/smistad/data/eyeguide/axillary_nerve_block/5/2016Dec30_082046/#.png",
            //"/home/smistad/data/eyeguide/axillary_nerve_block/5/2016Dec30_082110/#.png",
    };
    subjects[17] = {
            "/home/smistad/data/eyeguide/axillary_nerve_block/17/2017Feb13_150433/#.png",
            "/home/smistad/data/eyeguide/axillary_nerve_block/17/2017Feb13_150648/#.png",
            "/home/smistad/data/eyeguide/axillary_nerve_block/17/2017Feb13_150824/#.png",
    };
    subjects[22] = {
            "/home/smistad/data/eyeguide/axillary_nerve_block/22/2017-03-29-101035/US-2D_#.mhd",
            "/home/smistad/data/eyeguide/axillary_nerve_block/22/2017-03-29-101340/US-2D_#.mhd",
    };
     */
    subjects[30] = {
            "/home/smistad/data/eyeguide/axillary_nerve_block/30/2017Apr25_103134/#.png",
    };
    for(auto item : subjects) {
        ImageFileStreamer::pointer streamer = ImageFileStreamer::New();
        streamer->setFilenameFormats(item.second);
        streamer->setStartNumber(1);
        streamer->setSleepTime(45);

        PixelClassifier::pointer segmentation = PixelClassifier::New();
        segmentation->setNrOfClasses(6);
        segmentation->load("/home/smistad/workspace/eyeguide_keras/models/axillary_block_" + std::to_string(item.first) + ".pb");
        segmentation->setInputSize(256, 256);
        segmentation->setScaleFactor(1.0f / 255.0f);
        segmentation->setOutputParameters({"conv2d_23/truediv"});
        segmentation->setInputConnection(streamer->getOutputPort());
        segmentation->setHeatmapOutput();
        segmentation->setPreserveAspectRatio(true);
        segmentation->enableRuntimeMeasurements();
        segmentation->setHorizontalFlipping(true);

        HeatmapRenderer::pointer renderer = HeatmapRenderer::New();
        renderer->addInputConnection(segmentation->getOutputPort(1), Color::Red());
        renderer->addInputConnection(segmentation->getOutputPort(2), Color::Yellow());
        renderer->addInputConnection(segmentation->getOutputPort(3), Color::Green());
        renderer->addInputConnection(segmentation->getOutputPort(4), Color::Purple());
        renderer->addInputConnection(segmentation->getOutputPort(5), Color::Cyan());
        renderer->setMaxOpacity(0.3);
        renderer->setMinConfidence(0.2);
        renderer->enableRuntimeMeasurements();

        ImageRenderer::pointer renderer2 = ImageRenderer::New();
        renderer2->setInputConnection(streamer->getOutputPort());

        SimpleWindow::pointer window = SimpleWindow::New();

        window->addRenderer(renderer2);
        window->addRenderer(renderer);
        window->getView()->enableRuntimeMeasurements();
        window->setSize(1920, 1080);
        //window->enableFullscreen();
        if(item.second[0].find(".png") != std::string::npos)
            window->getView()->set2DPixelSpacing(0.3);
        window->set2DMode();
        window->getView()->setBackgroundColor(Color::Black());
        window->start();

        segmentation->getAllRuntimes()->printAll();
        renderer->getAllRuntimes()->printAll();
        window->getView()->getAllRuntimes()->printAll();
    }
}
