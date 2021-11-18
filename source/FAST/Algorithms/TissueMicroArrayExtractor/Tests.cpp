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
}*/

/*
TEST_CASE("Tissue micro array extractor", "[fast][wsi][tma][TissueMicroArrayExtractor]") {
    auto importer = WholeSlideImageImporter::create("/home/smistad/data/TMA/H2 TP02 HE helsnittscan_plane_0_cm_lzw_jpeg_Q_85.tif");
    //auto importer = WholeSlideImageImporter::create("/home/smistad/data/TMA/TP02  pan CK AE1-AE3_01_plane_0_cm_lzw_jpeg_Q_85.tif");

    auto extractor = TissueMicroArrayExtractor::create(2)->connect(importer);

    auto renderer = ImageRenderer::create()->connect(extractor);

    SimpleWindow2D::create()->connect(renderer)->run();
}
*/