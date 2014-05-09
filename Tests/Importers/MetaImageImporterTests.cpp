#include "catch.hpp"
#include "MetaImageImporter.hpp"

using namespace fast;

TEST_CASE("No filename", "[fast][MetaImageImporter]") {
    MetaImageImporter::pointer importer = MetaImageImporter::New();
    CHECK_THROWS(importer->update());
}

TEST_CASE("Import non-existing file", "[fast][MetaImageImporter]") {
    MetaImageImporter::pointer importer = MetaImageImporter::New();
    importer->setFilename("asdasdasdsad");
    CHECK_THROWS_AS(importer->update(), FileNotFoundException);
}

TEST_CASE("Import MetaImage file to host", "[fast][MetaImageImporter]") {
    MetaImageImporter::pointer importer = MetaImageImporter::New();
    importer->setFilename("/home/smistad/Patients/2013-08-22_10-36_Lab_4DTrack.cx3/US_Acq/US-Acq_01_20130822T111033/US-Acq_01_20130822T111033_ScanConverted_0.mhd");
    importer->setDevice(Host::New());
    Image::pointer image = importer->getOutput();
    image->update();

    // Check attributes of image
    CHECK(image->getWidth() == 276);
    CHECK(image->getHeight() == 249);
    CHECK(image->getDepth() == 200);
    CHECK(image->getDimensions() == 3);
    CHECK(image->getDataType() == TYPE_UINT8);
}

TEST_CASE("Import MetaImage file to OpenCL device", "[fast][MetaImageImporter]") {
    MetaImageImporter::pointer importer = MetaImageImporter::New();
    importer->setFilename("/home/smistad/Patients/2013-08-22_10-36_Lab_4DTrack.cx3/US_Acq/US-Acq_01_20130822T111033/US-Acq_01_20130822T111033_ScanConverted_0.mhd");
    Image::pointer image = importer->getOutput();
    image->update();

    // Check attributes of image
    CHECK(image->getWidth() == 276);
    CHECK(image->getHeight() == 249);
    CHECK(image->getDepth() == 200);
    CHECK(image->getDimensions() == 3);
    CHECK(image->getDataType() == TYPE_UINT8);
}

