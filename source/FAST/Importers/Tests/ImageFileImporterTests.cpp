#include "FAST/Testing.hpp"
#include "FAST/Importers/ImageFileImporter.hpp"

using namespace fast;

TEST_CASE("ImageFileImporter MetaImage", "[fast][ImageFileImporter]") {
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(Config::getTestDataPath()+"US/CarotidArtery/Right/US-2D_0.mhd");
    CHECK_NOTHROW(importer->update());
}

TEST_CASE("ImageFileImporter JPG", "[fast][ImageFileImporter]") {
    ImageFileImporter::pointer importer = ImageFileImporter::New();
    importer->setFilename(Config::getTestDataPath()+"US/US-2D.jpg");
    CHECK_NOTHROW(importer->update());
}
