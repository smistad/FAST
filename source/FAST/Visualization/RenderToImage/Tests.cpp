#include <FAST/Testing.hpp>
#include <FAST/Visualization/RenderToImage/RenderToImage.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Importers/ImageFileImporter.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Algorithms/BinaryThresholding/BinaryThresholding.hpp>
#include <FAST/Visualization/SegmentationRenderer/SegmentationRenderer.hpp>
#include <FAST/Exporters/ImageExporter.hpp>
#include <FAST/Streamers/ImageFileStreamer.hpp>
#include <FAST/Visualization/VolumeRenderer/MaximumIntensityProjection.hpp>
#include <FAST/Visualization/SliceRenderer/SliceRenderer.hpp>
#include <FAST/Visualization/VolumeRenderer/ThresholdVolumeRenderer.hpp>
#include <FAST/Visualization/ImagePyramidRenderer/ImagePyramidRenderer.hpp>
#include <FAST/Importers/WholeSlideImageImporter.hpp>
#include <FAST/Algorithms/ImagePatch/PatchGenerator.hpp>
#include <FAST/Algorithms/TissueSegmentation/TissueSegmentation.hpp>
#include <FAST/Algorithms/ImagePatch/PatchStitcher.hpp>
#include <FAST/Algorithms/RunUntilFinished/RunUntilFinished.hpp>

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

TEST_CASE("RenderToImage 3D", "[fast][RenderToImage]") {
    auto importer = ImageFileImporter::create(Config::getTestDataPath() + "CT/CT-Thorax.mhd");

    auto renderer = MaximumIntensityProjection::create()->connect(importer);

    //SimpleWindow3D::create()->connect(renderer)->run();
    auto toImage = RenderToImage::create(Color::White(), 1024 )->connect(renderer);
    ImageExporter::create("test_render_to_image_3d_volume.png")->connect(toImage)->run();
}

TEST_CASE("RenderToImage 3D volume + geom", "[fast][RenderToImage]") {
    auto importer = ImageFileImporter::create(Config::getTestDataPath() + "CT/CT-Thorax.mhd");

    auto renderer = ThresholdVolumeRenderer::create(300)->connect(importer);

    auto renderer2 = SliceRenderer::create(PLANE_Z)->connect(importer);

    //SimpleWindow3D::create()->connect({renderer, renderer2})->run();
    auto toImage = RenderToImage::create(Color::White(), 1024 )->connect({renderer, renderer2});
    ImageExporter::create("test_render_to_image_3d_geom_volume.png")->connect(toImage)->run();
}

TEST_CASE("RenderToImage 3D geom", "[fast][RenderToImage]") {
    auto importer = ImageFileImporter::create(Config::getTestDataPath() + "CT/CT-Thorax.mhd");

    auto renderer2 = SliceRenderer::create(PLANE_Z)->connect(importer);

    //SimpleWindow3D::create()->connect(renderer2)->run();
    auto toImage = RenderToImage::create(Color::White(), 1024 )->connect(renderer2);
    ImageExporter::create("test_render_to_image_3d_geom.png")->connect(toImage)->run();
}
TEST_CASE("RenderToImage image pyramid", "[fast][RenderToImage]") {
    auto importer = WholeSlideImageImporter::create(Config::getTestDataPath() + "/WSI/CMU-1.svs");

    auto renderer = ImagePyramidRenderer::create()->connect(importer);

    auto toImage = RenderToImage::create()->connect(renderer);
    auto image = toImage->runAndGetOutputData<Image>();

    //auto renderer2 = ImageRenderer::create()->connect(image);
    //SimpleWindow2D::create()->connect(renderer2)->run();

    ImageExporter::create("test_render_to_image_image_pyramid.png")->connect(image)->run();
}

TEST_CASE("RenderToImage image pyramid + segmentation", "[fast][RenderToImage]") {
    auto importer = WholeSlideImageImporter::create(Config::getTestDataPath() + "/WSI/CMU-1.svs");

    auto generator = PatchGenerator::create(512, 512, 1, 1)->connect(importer);

    auto segmentation = TissueSegmentation::create()->connect(generator);

    auto stitcher = PatchStitcher::create()->connect(segmentation);

    auto finish = RunUntilFinished::create()->connect(stitcher);

    auto renderer = ImagePyramidRenderer::create()->connect(importer);

    auto renderer2 = SegmentationRenderer::create()->connect(finish);

    auto toImage = RenderToImage::create()->connect({renderer, renderer2});
    auto image = toImage->runAndGetOutputData<Image>();

    //auto renderer3 = ImageRenderer::create()->connect(image);
    //SimpleWindow2D::create()->connect(renderer3)->run();

    ImageExporter::create("test_render_to_image_image_pyramid_and_segmentation.png")->connect(image)->run();
}
