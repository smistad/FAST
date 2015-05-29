#include "FAST/Tests/catch.hpp"
#include "FAST/Importers/ImageFileImporter.hpp"

using namespace fast;

TEST_CASE("ImageFileImporter MetaImage", "[fast][ImageFileImporter]") {
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR)+"US-2Dt/US-2Dt_0.mhd");
    CHECK_NOTHROW(importer->update());
}

TEST_CASE("ImageFileImporter JPG", "[fast][ImageFileImporter]") {
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR)+"US-2D.jpg");
    CHECK_NOTHROW(importer->update());
}
