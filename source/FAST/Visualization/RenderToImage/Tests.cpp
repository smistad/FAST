#include <FAST/Testing.hpp>
#include <FAST/Visualization/RenderToImage/RenderToImage.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Importers/ImageFileImporter.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Algorithms/BinaryThresholding/BinaryThresholding.hpp>
#include <FAST/Visualization/SegmentationRenderer/SegmentationRenderer.hpp>
#include <FAST/Exporters/ImageExporter.hpp>
#include <FAST/Streamers/ImageFileStreamer.hpp>

using namespace fast;

TEST_CASE("RenderToImage", "[fast][RenderToImage]") {
    auto importer = ImageFileImporter::create(Config::getTestDataPath() + "US/US-2D.jpg");

    auto threshold = BinaryThresholding::create(50)->connect(importer);

    auto renderer = ImageRenderer::create()->connect(importer);

    auto segRenderer = SegmentationRenderer::create()->connect(threshold);

    auto toImage = RenderToImage::create()->connect({renderer, segRenderer});
    auto image = toImage->runAndGetOutputData<Image>();

    //auto renderer2 = ImageRenderer::create()->connect(image);
    //SimpleWindow2D::create()->connect(renderer2)->run();

    ImageExporter::create("test.png")->connect(image)->run();
}
TEST_CASE("RenderToImage on stream", "[fast][RenderToImage]") {
    auto importer = ImageFileStreamer::create(Config::getTestDataPath() + "US/Heart/ApicalFourChamber/US-2D_#.mhd");

    auto threshold = BinaryThresholding::create(50)->connect(importer);

    auto renderer = ImageRenderer::create()->connect(importer);

    auto segRenderer = SegmentationRenderer::create()->connect(threshold);

    auto toImage = RenderToImage::create()->connect({renderer, segRenderer});

    auto stream = DataStream(toImage);
    int timestep = 0;
    while(!stream.isDone()) {
        auto image = stream.getNextFrame<Image>();
        ImageExporter::create("test_" + std::to_string(timestep) + ".png")->connect(image)->run();
        ++timestep;
        //if(timestep == 4)
        //    break;
    }
}
