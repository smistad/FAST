#include <FAST/Exporters/ImagePyramidPatchExporter.hpp>
#include <FAST/Testing.hpp>
#include <FAST/Importers/WholeSlideImageImporter.hpp>

#include <FAST/Importers/ImagePyramidPatchImporter.hpp>
#include <FAST/Visualization/ImagePyramidRenderer/ImagePyramidRenderer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>

using namespace fast;

TEST_CASE("ImagePyramidPatchExporter", "[fast][ImagePyramidPatchExporter]") {
    std::string path = "patch_export_test";
    auto importer = WholeSlideImageImporter::create(Config::getTestDataPath() + "/WSI/A05.svs");
    auto exporter = ImagePyramidPatchExporter::create(path, 2, 512, 512);
    exporter->connect(importer)->run();

    /*
    auto importer2 = ImagePyramidPatchImporter::create(path);

    auto renderer = ImagePyramidRenderer::create()->connect(importer2);
    auto window = SimpleWindow2D::create()->connect(renderer);
    window->setTimeout(1000);
    window->run();
    */
}

/*
TEST_CASE("ImagePyramidPatchExporter streaming", "[fast][ImagePyramidPatchExporter]") {
    std::string path = "C:/data/patch_export_test_stream";
    auto importer = WholeSlideImageImporter::create(Config::getTestDataPath() + "/WSI/A05.svs");

    auto generator = PatchGenerator::create(512, 512, 2)->connect(importer);
    auto segment = BinaryThresholding::create()->connect(generator);

    auto exporter = ImagePyramidPatchExporter::create(path, 1, 512, 512);
    exporter->connect(importer)->run();

    auto importer2 = ImagePyramidPatchImporter::create(path);

    auto renderer = ImagePyramidRenderer::create()->connect(importer2);
    SimpleWindow2D::create()->connect(renderer)->run();
}*/

