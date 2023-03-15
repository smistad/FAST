#include "TissueMicroArrayExtractor.hpp"
#include <FAST/Testing.hpp>
#include <FAST/Importers/WholeSlideImageImporter.hpp>
#include <FAST/Visualization/ImagePyramidRenderer/ImagePyramidRenderer.hpp>
#include <FAST/Visualization/SegmentationRenderer/SegmentationRenderer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Visualization/VertexRenderer/VertexRenderer.hpp>
#include <FAST/Visualization/BoundingBoxRenderer/BoundingBoxRenderer.hpp>

using namespace fast;

/*
TEST_CASE("Tissue micro array extractor", "[fast][wsi][tma][TissueMicroArrayExtractor]") {
    auto importer = WholeSlideImageImporter::create("/home/smistad/data/TMA/H2 TP02 HE helsnittscan_plane_0_cm_lzw_jpeg_Q_85.tif");
    //auto importer = WholeSlideImageImporter::create("/home/smistad/data/TMA/TP02  pan CK AE1-AE3_01_plane_0_cm_lzw_jpeg_Q_85.tif");

    auto extractor = TissueMicroArrayExtractor::create()->connect(importer);

    auto imageRenderer = ImagePyramidRenderer::create()->connect(importer);
    auto segRenderer = SegmentationRenderer::create()->connect(extractor);
    auto pointRenderer = VertexRenderer::create()->connect(extractor, 1);
    auto bbRenderer = BoundingBoxRenderer::create(20.0f, {{1, Color::Blue()}})->connect(extractor, 2);

    SimpleWindow2D::create()->connect({imageRenderer, segRenderer, bbRenderer})->run();
}

TEST_CASE("Tissue micro array extractor", "[fast][wsi][tma][TissueMicroArrayExtractor]") {
    auto importer = WholeSlideImageImporter::create("/home/smistad/data/TMA/TMA_TA407.svs");
    //auto importer = WholeSlideImageImporter::create("/home/smistad/data/TMA/EFI test_TP02 panCK_AE1-AE3/TP02  pan CK AE1-AE3_01.vsi");

    auto extractor = TissueMicroArrayExtractor::create(0)->connect(importer);

    auto renderer = ImageRenderer::create()->connect(extractor);

    SimpleWindow2D::create()->connect(renderer)->run();
}

TEST_CASE("Tissue micro array extractor translation", "[fast][wsi][tma][TissueMicroArrayExtractor][visual]") {
    //auto importer = WholeSlideImageImporter::create("/home/smistad/data/TMA/TP02  pan CK AE1-AE3_01_plane_0_cm_lzw_jpeg_Q_85.tif");
    //auto importer = WholeSlideImageImporter::create("/home/smistad/data/TMA/EFI test_TP02 panCK_AE1-AE3/TP02  pan CK AE1-AE3_01.vsi");
    auto importer = WholeSlideImageImporter::create("/home/smistad/data/WSI/HUS_EFI_HE_BC_8.vsi");

    auto window = SimpleWindow2D::create();

    auto extractor = TissueMicroArrayExtractor::create(4)->connect(importer);
    auto stream = DataStream(extractor);
    int counter = 0;
    do {
        auto TMA = stream.getNextFrame<Image>();
        std::cout << TMA->getTransform(true)->getTranslation().transpose() << std::endl;
        auto renderer = ImageRenderer::create(-1, -1, 1.0f, true)->connect(TMA);
        window->connect(renderer);
        ++counter;
    } while(!stream.isDone());
    std::cout << "Count: " << counter << std::endl;

//    importer = WholeSlideImageImporter::create("/home/smistad/data/TMA/TP02 HE  helsnitt/H2 TP02 HE helsnittscan.vsi");
//    //importer = WholeSlideImageImporter::create("/home/smistad/data/TMA/H2 TP02 HE helsnittscan_plane_0_cm_lzw_jpeg_Q_85.tif");
//    extractor = TissueMicroArrayExtractor::create(4)->connect(importer);
//    stream = DataStream(extractor);
//    do {
//        auto TMA = stream.getNextFrame<Image>();
//        auto renderer = ImageRenderer::create(-1, -1, 0.5f, true)->connect(TMA);
//        window->connect(renderer);
//    } while(!stream.isDone());
//    window->run();
}*/