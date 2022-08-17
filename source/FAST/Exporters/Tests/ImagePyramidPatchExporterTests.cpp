#include <FAST/Exporters/ImagePyramidPatchExporter.hpp>
#include <FAST/Testing.hpp>
#include <FAST/Importers/WholeSlideImageImporter.hpp>

using namespace fast;

TEST_CASE("ImagePyramidPatchExporter", "[fast][ImagePyramidPatchExporter]") {
    auto importer = WholeSlideImageImporter::create(Config::getTestDataPath() + "/WSI/A05.svs");
    auto exporter = ImagePyramidPatchExporter::create("patch_export_test", 2, 512, 512);
    exporter->connect(importer)->run();
}
