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

class Subject {
    public:
        std::vector<std::string> mRecordings;
        bool mFlip;
        int mID;
        explicit Subject(int ID, const std::vector<std::string> recordings, bool flip = false) : mID(ID), mRecordings(recordings), mFlip(flip) {
        };
};

int main() {
    Reporter::setGlobalReportMethod(Reporter::COUT);
    std::vector<Subject> subjects;
    /*
    subjects[1] = {
            "/home/smistad/data/eyeguide/axillary_nerve_block/1/2016-10-07-135630/US-2D_#.mhd"
    };
    subjects[5] = {
            "/home/smistad/data/eyeguide/axillary_nerve_block/5/2016Dec30_082009/#.png",
            "/home/smistad/data/eyeguide/axillary_nerve_block/5/2016Dec30_082046/#.png",
            //"/home/smistad/data/eyeguide/axillary_nerve_block/5/2016Dec30_082110/#.png",
    };
     */
    /*
    subjects.push_back(Subject(6, {
            "/home/smistad/data/eyeguide/axillary/6/2016Dec30_084715/#.png",
    }));

    subjects.push_back(Subject(17, {
                    "/home/smistad/data/eyeguide/axillary/17/2017Feb13_150433/#.png",
                    "/home/smistad/data/eyeguide/axillary/17/2017Feb13_150648/#.png",
                    "/home/smistad/data/eyeguide/axillary/17/2017Feb13_150824/#.png",
    }));
     */

    subjects.push_back(Subject(18, {
            "/home/smistad/data/eyeguide/axillary/18/2017-03-29-094901/US-2D_#.mhd",
    }));

    subjects.push_back(Subject(30, {
            "/home/smistad/data/eyeguide/axillary/30/2017Apr25_103134/#.png",
    }, true));

    subjects.push_back(Subject(8, {
        "/home/smistad/data/eyeguide/axillary/8/2017Jan16_120434/#.png",
    }));
    subjects.push_back(Subject(13, {
        "/home/smistad/data/eyeguide/axillary/13/2017Feb13_090639/#.png",
    }));

    subjects.push_back(Subject(15, {
        "/home/smistad/data/eyeguide/axillary/15/2017Feb13_100740/#.png"
    }));
    subjects.push_back(Subject(41, {
        "/home/smistad/data/eyeguide/axillary/41/2017Nov28_094722/#.png"
    }, true));
    /*
   subjects[40] = {
       "/home/smistad/data/eyeguide/axillary_nerve_block/40/2017Nov28_090653/#.png",
   };
   */
    for(auto subject : subjects) {
        ImageFileStreamer::pointer streamer = ImageFileStreamer::New();
        streamer->setFilenameFormats(subject.mRecordings);
        streamer->setStartNumber(1);
        streamer->setSleepTime(50);

        PixelClassifier::pointer segmentation = PixelClassifier::New();
        segmentation->setHeatmapOutput();
        segmentation->setNrOfClasses(6);
        segmentation->load("/home/smistad/workspace/eyeguide_keras/models/axillary_block_" + std::to_string(subject.mID) + "_flip_gamma_rotate_shadow_elastic.pb");
        segmentation->setInputSize(256, 256);
        segmentation->setScaleFactor(1.0f / 255.0f);
        segmentation->setOutputParameters({"conv2d_23/truediv"});
        segmentation->setInputConnection(streamer->getOutputPort());
        //segmentation->setPreserveAspectRatio(true);
        segmentation->enableRuntimeMeasurements();
        segmentation->setHorizontalFlipping(subject.mFlip);

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
        renderer2->setInputConnection(streamer->getOutputPort());

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
}
