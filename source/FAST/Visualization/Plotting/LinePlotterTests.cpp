#include <FAST/Testing.hpp>
#include <FAST/Visualization/Plotting/LinePlotter.hpp>
#include <FAST/Algorithms/NeuralNetwork/SegmentationNetwork.hpp>
#include <FAST/Streamers/ImageFileStreamer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Visualization/SegmentationRenderer/SegmentationRenderer.hpp>
#include <FAST/Visualization/SegmentationLabelRenderer/SegmentationLabelRenderer.hpp>
#include <FAST/Algorithms/UltrasoundImageCropper/UltrasoundImageCropper.hpp>
#include <FAST/Visualization/Widgets/PlaybackWidget/PlaybackWidget.hpp>
#include <FAST/Algorithms/Lambda/RunLambda.hpp>
#include <FAST/Algorithms/RegionProperties/RegionProperties.hpp>

using namespace fast;

TEST_CASE("Line plotter", "[fast][LinePlotter]") {
    auto streamer = ImageFileStreamer::create(Config::getTestDataPath() + "US/JugularVein/US-2D_#.mhd");
    auto segmentation = SegmentationNetwork::create(join(Config::getTestDataPath(), "NeuralNetworkModels/jugular_vein_segmentation.onnx"), 1.0f/255.0f);
    segmentation->connect(streamer);

    auto segmentationRenderer = SegmentationRenderer::create({{1, Color::Red()}, {2, Color::Blue()}}, 0.25)
            ->connect(segmentation);

    auto area = RunLambda::create([](DataObject::pointer input) {
        auto segmentation = std::dynamic_pointer_cast<Image>(input);
        auto regions = RegionProperties::create()->connect(segmentation)->runAndGetOutputData<RegionList>();

        float area1 = 0;
        float area2 = 0;
        for(auto region : regions->get()) {
            if(region.label == 1) {
                area1 += region.area;
            } else if(region.label == 2) {
                area2 += region.area;
            }
        }

        return DataList({{0, FloatScalar::create(area1)}, {1, FloatScalar::create(area2)}});
    })->connect(segmentation);

    auto plotter = LinePlotter::create(200);
    plotter->connect(0, area, 0);
    plotter->connect(1, area, 1);

    auto imageRenderer = ImageRenderer::create()
            ->connect(streamer);

    auto window = SimpleWindow2D::create(Color::Black())
            ->connect({imageRenderer, segmentationRenderer});
    window->addWidget((QWidget*)plotter->getPlotterWidget());
    window->addProcessObject(plotter);
    window->setTimeout(2000);
    window->run();
}