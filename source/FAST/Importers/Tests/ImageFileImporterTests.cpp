#include "FAST/Testing.hpp"
#include "FAST/Importers/ImageFileImporter.hpp"

using namespace fast;

TEST_CASE("ImageFileImporter MetaImage", "[fast][ImageFileImporter]") {
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR)+"US/CarotidArtery/Right/US-2D_0.mhd");
    CHECK_NOTHROW(importer->update());
}

TEST_CASE("ImageFileImporter JPG", "[fast][ImageFileImporter]") {
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR)+"US/US-2D.jpg");
    CHECK_NOTHROW(importer->update());
}
