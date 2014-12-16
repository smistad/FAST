#include "catch.hpp"
#include "ImageImporter.hpp"
#include "DeviceManager.hpp"

using namespace fast;

TEST_CASE("No filename set to ImageImporter", "[fast][ImageImporter]") {
    ImageImporter::pointer importer = ImageImporter::New();
    CHECK_THROWS(importer->update());
}

TEST_CASE("Import non-existing file to ImageImporter", "[fast][ImageImporter]") {
    ImageImporter::pointer importer = ImageImporter::New();
    importer->setFilename("asdasdasdsad");
    CHECK_THROWS_AS(importer->update(), FileNotFoundException);
}

TEST_CASE("Import Image file to host", "[fast][ImageImporter]") {
    ImageImporter::pointer importer = ImageImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR) + "US-2D.jpg");
    importer->setMainDevice(Host::getInstance());
    Image::pointer image = importer->getOutput();
    image->update();

    // Check attributes of image
    CHECK(image->getWidth() == 512);
    CHECK(image->getHeight() == 512);
    CHECK(image->getDepth() == 1);
    CHECK(image->getDimensions() == 2);
    CHECK(image->getDataType() == TYPE_FLOAT);
}

TEST_CASE("Import Image file to OpenCL device", "[fast][MetaImageImporter]") {
    DeviceManager& deviceManager = DeviceManager::getInstance();
    OpenCLDevice::pointer device = deviceManager.getOneOpenCLDevice();

    ImageImporter::pointer importer = ImageImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR) + "US-2D.jpg");
    importer->setMainDevice(device);
    Image::pointer image = importer->getOutput();
    image->update();

    // Check attributes of image
    CHECK(image->getWidth() == 512);
    CHECK(image->getHeight() == 512);
    CHECK(image->getDepth() == 1);
    CHECK(image->getDimensions() == 2);
    CHECK(image->getDataType() == TYPE_FLOAT);
}

